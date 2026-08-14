/*
 * macro_processor.c
 * Macro processor (pre-assembler) implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "utils.h"
#include "macro_processor.h"
#include "macro_table.h"
#include "logger.h"
#include "growable_array.h"


Boolean isMcroDefinitionStart(const char *line, char *name_out)
{
    char *token;
    const char *rest;

    if (line == NULL || name_out == NULL) {
        return FALSE;
    }

    /* First token must be "mcro" */
    token = getFirstToken(line, " \t", &rest);
    if (token == NULL || strcmp(token, "mcro") != 0) {
        free(token);
        return FALSE;
    }
    free(token);
    rest = skipWhitespace(rest);

    /* Second token is the macro name */
    token = getFirstToken(rest, " \t", &rest);
    if (token == NULL || token[0] == '\0') {
        free(token);
        return FALSE;  /* No macro name provided */
    }

    /* Reject before copying so an oversized name can never overflow name_out */
    if (strlen(token) > MAX_MACRO_NAME_LENGTH) {
        free(token);
        return FALSE;
    }

    strcpy(name_out, token);
    free(token);
    rest = skipWhitespace(rest);

    /* There should be nothing left */
    if (*rest != '\0') {
        return FALSE;  /* Extra words after macro name */
    }

    return TRUE;
}

Boolean isMcroDefinitionEnd(const char *line)
{
    char *token;
    const char *rest;

    if (line == NULL) {
        return FALSE;
    }

    /* First token must be "mcroend" */
    token = getFirstToken(line, " \t", &rest);
    if (token == NULL || strcmp(token, "mcroend") != 0) {
        free(token);
        return FALSE;
    }
    free(token);
    rest = skipWhitespace(rest);

    /* There should be nothing left */
    if (*rest != '\0') {
        return FALSE;  /* Extra words after mcroend */
    }

    return TRUE;
}


/**
 * Doubles the capacity of the macro body buffer.
 * @param macro_body pointer to the buffer to grow
 * @param capacity pointer to its current capacity, updated on success
 * @return TRUE on success, FALSE on allocation failure (caller must clean up)
 */
static Boolean resizeMacroBuffer(char **macro_body, int *capacity)
{
    return growArray((void **)macro_body, capacity, sizeof(char));
}

/**
 * Opens the input/output files, creates the macro table, and initializes state.
 * @param input_file path to the source .as file
 * @param output_file path to the .am file to create
 * @param state the state struct to initialize
 * @return RC_SUCCESS, RC_IO_ERROR if either file couldn't be opened, or
 *         RC_OUT_OF_MEMORY if the macro table couldn't be allocated
 */
static ResultCode initializeResources(
    const char *input_file, const char *output_file,
    MacroProcessorState *state)
{
    state->in_fp = fopen(input_file, "r");
    if (state->in_fp == NULL) {
        logMessage(LOG_ERROR, ERR_MACRO_CANNOT_OPEN_INPUT, input_file);
        return RC_IO_ERROR;
    }

    state->out_fp = fopen(output_file, "w");
    if (state->out_fp == NULL) {
        logMessage(LOG_ERROR, ERR_MACRO_CANNOT_OPEN_OUTPUT, output_file);
        fclose(state->in_fp);
        return RC_IO_ERROR;
    }

    state->macro_table = createMacroTable();
    if (state->macro_table == NULL) {
        logMessage(LOG_ERROR, ERR_MACRO_TABLE_ALLOC_FAILED);
        fclose(state->in_fp);
        fclose(state->out_fp);
        return RC_OUT_OF_MEMORY;
    }

    /* Initialize state fields */
    state->line_num = 0;
    state->is_error = 0;
    state->in_macro = 0;
    state->macro_body = NULL;
    state->macro_capacity = 0;
    state->macro_length = 0;
    state->name[0] = '\0';

    return RC_SUCCESS;
}

/**
 * Strips the trailing newline, surrounding whitespace, and any comment from
 * a line, in place.
 * @param line the line to preprocess
 */
static void preprocessLine(char *line)
{
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
        line[strlen(line) - 1] = '\0';
    }
    removeComments(line);
    trimWhitespace(line);
}

/**
 * Appends a line to the macro body buffer, growing it if needed.
 * @param state holds the buffer being built
 * @param line the line to append
 * @return TRUE on success, FALSE on allocation failure
 */
static Boolean appendToMacroBody(
    MacroProcessorState *state, const char *line)
{
    int needed = state->macro_length + strlen(line) + 2;

    while (needed > state->macro_capacity) {
        if (!resizeMacroBuffer(&state->macro_body, &state->macro_capacity)) {
            logMessage(LOG_ERROR, ERR_MACRO_BODY_RESIZE_FAILED);
            return FALSE;
        }
    }

    strcat(state->macro_body, line);
    strcat(state->macro_body, "\n");
    state->macro_length = strlen(state->macro_body);
    return TRUE;
}

/**
 * Handles an "mcroend" line: registers the collected macro body into the
 * macro table.
 * @param state the current processing state
 * @return TRUE to continue processing, FALSE to stop (error occurred)
 */
static Boolean handleMcroEnd(MacroProcessorState *state)
{
    ResultCode insert_result;

    /* Full validation lives here, mirroring handleMcroStart below. */
    if (!isMcroDefinitionEnd(state->line)) {
        logMessage(LOG_ERROR, ERR_MCROEND_TRAILING_CHARS, state->line_num);
        return FALSE;  /* Break */
    }

    if (!state->in_macro) {
        logMessage(LOG_ERROR, ERR_MCROEND_WITHOUT_MCRO, state->line_num);
        return FALSE;  /* Break */
    }

    state->in_macro = 0;

    insert_result = insertMacro(state->macro_table, state->name, state->macro_body);

    switch (insert_result) {
        case RC_SUCCESS:
            logMessage(LOG_DEBUG, "Line %d: End of macro '%s'\n", state->line_num, state->name);
            logMessage(LOG_DEBUG, "Macro '%s' registered in table\n", state->name);

            /* Ownership of macro_body has passed to the macro table's node;
             * drop our reference so finalizeResources() doesn't free it a
             * second time. */
            state->macro_body = NULL;
            state->macro_capacity = 0;
            state->macro_length = 0;
            return TRUE;  /* Continue */
        case RC_DUPLICATE:
            logMessage(LOG_ERROR, ERR_MACRO_ALREADY_DEFINED, state->line_num, state->name);
            break;
        case RC_OUT_OF_MEMORY:
            logMessage(LOG_ERROR, ERR_MACRO_REGISTER_ALLOC_FAILED, state->name);
            break;
        default:
            logMessage(LOG_ERROR, ERR_MACRO_REGISTER_UNEXPECTED, state->name);
            break;
    }

    free(state->macro_body);
    state->macro_body = NULL;
    return FALSE;  /* Break */
}

/**
 * Handles an "mcro <name>" line: validates the syntax and name, then
 * initializes the macro body buffer.
 * @param state the current processing state
 * @return TRUE to continue processing, FALSE to stop (error occurred)
 */
static Boolean handleMcroStart(MacroProcessorState *state)
{
    /* Rejected outright if we're already inside an unterminated macro,
     * instead of silently starting a new one and leaking/discarding the one
     * in progress -- mirrors handleMcroEnd's own !state->in_macro check. */
    if (state->in_macro) {
        logMessage(LOG_ERROR, ERR_MACRO_NESTED_DEFINITION, state->line_num, state->name);
        return FALSE;  /* Break */
    }

    if (!isMcroDefinitionStart(state->line, state->name)) {
        logMessage(LOG_ERROR, ERR_MCRO_INVALID_SYNTAX, state->line_num);
        return FALSE;  /* Break */
    }

    /* Validate macro name follows label syntax: letter, then alphanumeric */
    if (!isValidSymbolName(state->name)) {
        logMessage(LOG_ERROR, ERR_MACRO_NAME_INVALID, state->line_num, state->name);
        return FALSE;  /* Break */
    }

    /* Validate macro name is not reserved */
    if (isReservedWord(state->name)) {
        logMessage(LOG_ERROR, ERR_MACRO_NAME_RESERVED, state->line_num, state->name);
        return FALSE;  /* Break */
    }

    logMessage(LOG_DEBUG, "Line %d: Found macro definition 'mcro %s'\n",
               state->line_num, state->name);

    /* Initialize macro body buffer */
    state->macro_capacity = INITIAL_MACRO_BODY_CAPACITY;
    state->macro_length = 0;
    state->macro_body = (char *)malloc(state->macro_capacity);

    if (state->macro_body == NULL) {
        logMessage(LOG_ERROR, ERR_MACRO_BODY_ALLOC_FAILED);
        return FALSE;  /* Break */
    }

    state->macro_body[0] = '\0';  /* Initialize as empty string */
    state->in_macro = 1;
    return TRUE;  /* Continue */
}

/**
 * Handles a line that's neither a macro definition nor inside one: writes it
 * through as-is, or expands it if it's a call to a known macro.
 * @param state the current processing state
 * @return TRUE to continue processing
 */
static Boolean handleRegularLine(MacroProcessorState *state)
{
    MacroNode *macro = lookupMacro(state->macro_table, state->line);
    if (macro != NULL) {
        logMessage(LOG_DEBUG, "Line %d: Expanding macro '%s'\n", state->line_num, state->line);
        fprintf(state->out_fp, "%s", macro->body);
    } else {
        fprintf(state->out_fp, "%s\n", state->line);
    }
    return TRUE;  /* Continue */
}

/**
 * Classifies one line (mcroend / mcro start / macro body / regular) and
 * dispatches to the matching handler.
 * @param state the current processing state
 * @return TRUE to continue processing, FALSE to stop (fatal error)
 */
static Boolean processLine(MacroProcessorState *state)
{
    char *first_word;

    /* Routing peek only; handlers own their own validation. Delimiter set
     * matches isspace(), as this call site used before getFirstToken. */
    first_word = getFirstToken(state->line, " \t\r\v\f", NULL);
    if (first_word == NULL) {
        logMessage(LOG_ERROR, ERR_MACRO_LINE_ALLOC_FAILED, state->line_num);
        return FALSE;  /* Break */
    }

    if (strcmp(first_word, "mcroend") == 0) {
        free(first_word);
        return handleMcroEnd(state);
    }

    if (strcmp(first_word, "mcro") == 0) {
        free(first_word);
        return handleMcroStart(state);
    }
    free(first_word);

    /* Inside a macro: collect lines */
    if (state->in_macro) {
        if (!appendToMacroBody(state, state->line)) {
            free(state->macro_body);
            state->macro_body = NULL;
            return FALSE;  /* Break */
        }
        return TRUE;  /* Continue */
    }

    /* Regular line or macro call */
    return handleRegularLine(state);
}

/**
 * Closes files and removes the partial output file if any errors occurred.
 * The macro table is only freed here on failure -- on success the caller
 * takes ownership of it (see expandMacros's macros_out).
 * @param state the current processing state
 * @param output_file path to the .am file, removed if had_errors is set
 * @param had_errors 1 if any error occurred while processing this file
 */
static void finalizeResources(
    MacroProcessorState *state, const char *output_file, int had_errors)
{
    if (had_errors && state->macro_table != NULL) {
        destroyMacroTable(state->macro_table);
    }
    if (state->in_fp != NULL) {
        fclose(state->in_fp);
    }
    if (state->out_fp != NULL) {
        fclose(state->out_fp);
    }
    if (state->macro_body != NULL) {
        free(state->macro_body);
        state->macro_body = NULL;
    }
    
    if (had_errors && remove(output_file) == 0) {
        logMessage(LOG_DEBUG, "Cleaned up partial output file: %s\n", output_file);
    }
}

ResultCode expandMacros(const char *input_file, const char *output_file, MacroTable **macros_out)
{
    MacroProcessorState state;
    char *table_str;
    ResultCode init_result;

    *macros_out = NULL;

    /* Initialize resources */
    init_result = initializeResources(input_file, output_file, &state);
    if (init_result != RC_SUCCESS) {
        return init_result;  /* RC_IO_ERROR or RC_OUT_OF_MEMORY */
    }
    
    logMessage(LOG_DEBUG, "=== MACRO PROCESSOR START ===\n");
    logMessage(LOG_DEBUG, "Input: %s\n", input_file);
    logMessage(LOG_DEBUG, "Output: %s\n", output_file);

    /* Main processing loop */
    while (fgets(state.line, LINE_BUFFER_SIZE, state.in_fp) != NULL) {
        state.line_num++;

        /* Checked on the raw, just-read line: isLineTooLong needs to see
         * whether a trailing newline was captured, which preprocessLine is
         * about to strip. */
        if (isLineTooLong(state.line)) {
            logMessage(LOG_ERROR, ERR_LINE_TOO_LONG, state.line_num, MAX_LINE_LENGTH);
            state.is_error = 1;
            break;
        }

        /* Preprocess: remove newline, trim, remove comments */
        preprocessLine(state.line);
        
        /* Skip empty lines */
        if (strlen(state.line) == 0) {
            continue;
        }
        
        /* Process the line and check for continuance */
        if (!processLine(&state)) {
            state.is_error = 1;
            break;  /* Fatal error, stop processing */
        }
    }
    
    /* Check for unclosed macro at EOF */
    if (state.in_macro) {
        logMessage(LOG_ERROR, ERR_MACRO_UNCLOSED, state.name);
        state.is_error = 1;
        if (state.macro_body != NULL) {
            free(state.macro_body);
            state.macro_body = NULL;
        }
    }

    /* Log final state  */
    if (state.is_error) {
        logMessage(LOG_DEBUG, "=== MACRO PROCESSOR FAILED ===\n");
    } else {
        /* Log final macro table and completion info */
        table_str = macroTableToString(state.macro_table);
        logMessage(LOG_DEBUG, "\n=== FINAL MACRO TABLE ===\n%s", table_str);
        free(table_str);
        logMessage(LOG_DEBUG, "=== MACRO PROCESSOR COMPLETE ===\n");
        logMessage(LOG_DEBUG, "Lines processed: %d\n", state.line_num);
    }
    
    /* Cleanup resources, delete output on error */
    finalizeResources(&state, output_file, state.is_error);

    if (!state.is_error) {
        *macros_out = state.macro_table;
    }

    return state.is_error ? RC_INVALID : RC_SUCCESS;
}
