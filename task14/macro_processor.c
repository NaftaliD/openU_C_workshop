/*
 * macro_processor.c
 * Macro processor (pre-assembler) implementation.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "utils.h"
#include "macro_processor.h"
#include "macro_table.h"
#include "logger.h"


int isMcroDefinitionStart(const char *line, char *name_out)
{
    char temp_line[MAX_LINE_LENGTH + 1];
    char *token;

    if (line == NULL || name_out == NULL) {
        return 0;
    }

    /* Make a copy to avoid modifying original */
    strcpy(temp_line, line);

    /* First token must be "mcro" */
    token = strtok(temp_line, " \t");
    if (token == NULL || strcmp(token, "mcro") != 0) {
        return 0;
    }

    /* Second token is the macro name */
    token = strtok(NULL, " \t");
    if (token == NULL) {
        return 0;  /* No macro name provided */
    }

    /* Reject before copying so an oversized name can never overflow name_out */
    if (strlen(token) > MAX_MACRO_NAME_LENGTH) {
        return 0;
    }

    strcpy(name_out, token);

    /* There should be no third token */
    token = strtok(NULL, " \t");
    if (token != NULL) {
        return 0;  /* Extra words after macro name */
    }

    return 1;
}

int isMcroDefinitionEnd(const char *line)
{
    char temp_line[MAX_LINE_LENGTH + 1];
    char *token;
    
    if (line == NULL) {
        return 0;
    }
    
    strcpy(temp_line, line);
    
    /* First token must be "mcroend" */
    token = strtok(temp_line, " \t");
    if (token == NULL || strcmp(token, "mcroend") != 0) {
        return 0;
    }
    
    /* There should be no second token */
    token = strtok(NULL, " \t");
    if (token != NULL) {
        return 0;  /* Extra words after mcroend */
    }
    
    return 1;
}


/*
 * Extract the first word/token from a line (up to space/tab or end of line).
 * Returns a malloc'd string containing the first word.
 * Caller must free the returned string.
 * Returns NULL if line is empty or contains no word.
 */
static char* get_first_word(const char *line)
{
    int i = 0;
    char *word;
    
    if (line == NULL || strlen(line) == 0) {
        return NULL;
    }
    
    /* Find where the first word ends (at space/tab or end of string) */
    while (line[i] != '\0' && !isspace((unsigned char)line[i])) {
        i++;
    }
    
    if (i == 0) {
        return NULL;  /* No word found */
    }
    
    /* Allocate and copy the first word */
    word = (char *)malloc(i + 1);
    if (word == NULL) {
        return NULL;
    }
    
    strncpy(word, line, i);
    word[i] = '\0';
    
    return word;
}

/*
 * Resize the macro body buffer when capacity is exceeded.
 * Returns: 1 on success, 0 on failure (caller must cleanup)
 */
static int resize_macro_buffer(char **macro_body, int *capacity, int *length)
{
    char *temp;
    
    if (macro_body == NULL || capacity == NULL) {
        return 0;
    }
    
    *capacity = *capacity * 2;
    temp = (char *)realloc(*macro_body, *capacity);
    
    if (temp == NULL) {
        return 0;  /* Allocation failed */
    }
    
    *macro_body = temp;
    return 1;
}

/*
 * Handle macro definition start (mcro <name>).
 * Initializes macro collection state and validates macro name.
 * Returns: 1 on success, 0 on error
 */
static int handle_mcro_start(const char *line, int line_num, char *name_out,
                             char **macro_body, int *capacity, int *length)
{
    if (!isMcroDefinitionStart(line, name_out)) {
        fprintf(stderr, "Error at line %d: Invalid macro definition syntax. "
                "Expected: mcro <name>\n", line_num);
        logMessage("ERROR line %d: Invalid macro syntax\n", line_num);
        return 0;
    }

    /* Validate macro name follows label syntax: letter, then alphanumeric */
    if (!isValidSymbolName(name_out)) {
        fprintf(stderr, "Error at line %d: Macro name '%s' is invalid "
                "(must start with a letter and contain only letters/digits)\n",
                line_num, name_out);
        logMessage("ERROR line %d: Macro name '%s' has invalid syntax\n", line_num, name_out);
        return 0;
    }

    /* Validate macro name is not reserved */
    if (isReservedWord(name_out)) {
        fprintf(stderr, "Error at line %d: Macro name '%s' is a reserved word\n",
                line_num, name_out);
        logMessage("ERROR line %d: Macro name '%s' is reserved\n", line_num, name_out);
        return 0;
    }

    logMessage("Line %d: Found macro definition 'mcro %s'\n", line_num, name_out);
    
    /* Initialize macro body buffer */
    *capacity = 256;
    *length = 0;
    *macro_body = (char *)malloc(*capacity);
    
    if (*macro_body == NULL) {
        fprintf(stderr, "Error: Cannot allocate macro body buffer\n");
        logMessage("ERROR: Cannot allocate macro body buffer\n");
        return 0;
    }
    
    (*macro_body)[0] = '\0';  /* Initialize as empty string */
    return 1;
}

/*
 * Handle macro definition end (mcroend).
 * Stores the collected macro into the macro table.
 * Returns: 1 on success, 0 on error (caller should cleanup macro_body)
 */
static int handle_mcro_end(int line_num, const char *name,
                          MacroTable *table, char *macro_body)
{
    if (!insertMacro(table, name, macro_body)) {
        logMessage("Line %d: End of macro '%s'\n", line_num, name);
        logMessage("Macro '%s' registered in table\n", name);
        return 1;
    }
    
    /* Macro already exists */
    fprintf(stderr, "Error at line %d: Macro '%s' already defined\n", 
            line_num, name);
    logMessage("ERROR line %d: Macro '%s' already defined\n", line_num, name);
    
    /* Return 0 to signal caller that cleanup is needed */
    return 0;
}


/*
 * Initialize file handles, macro table, and state.
 * Returns: 0 on success, -1 on I/O failure
 */
static int initializeResources(
    const char *input_file, const char *output_file,
    MacroProcessorState *state)
{
    state->in_fp = fopen(input_file, "r");
    if (state->in_fp == NULL) {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_file);
        return -1;
    }
    
    state->out_fp = fopen(output_file, "w");
    if (state->out_fp == NULL) {
        fprintf(stderr, "Error: Cannot open output file %s\n", output_file);
        fclose(state->in_fp);
        return -1;
    }
    
    state->macro_table = createMacroTable();
    if (state->macro_table == NULL) {
        fprintf(stderr, "Error: Cannot allocate macro table\n");
        fclose(state->in_fp);
        fclose(state->out_fp);
        return -1;
    }
    
    /* Initialize state fields */
    state->line_num = 0;
    state->is_error = 0;
    state->in_macro = 0;
    state->macro_body = NULL;
    state->macro_capacity = 0;
    state->macro_length = 0;
    state->name[0] = '\0';
    
    return 0;
}

/*
 * Preprocess a line: remove newline, trim whitespace, remove comments.
 */
static void preprocessLine(char *line)
{
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
        line[strlen(line) - 1] = '\0';
    }
    trimWhitespace(line);
    removeComments(line);
}

/*
 * Append a line to the macro body buffer.
 * Resizes buffer if needed.
 * Returns: 1 on success, 0 on error
 */
static int appendToMacroBody(
    MacroProcessorState *state, const char *line)
{
    int needed = state->macro_length + strlen(line) + 2;
    
    while (needed > state->macro_capacity) {
        if (!resize_macro_buffer(&state->macro_body, &state->macro_capacity,
                                &state->macro_length)) {
            fprintf(stderr, "Error: Cannot resize macro body buffer\n");
            logMessage("ERROR: Cannot resize macro body buffer\n");
            return 0;
        }
    }
    
    strcat(state->macro_body, line);
    strcat(state->macro_body, "\n");
    state->macro_length = strlen(state->macro_body);
    return 1;
}

/*
 * Handle mcroend directive.
 * Returns: 1 to continue processing, 0 to break (error occurred)
 */
static int handleMcroEnd(MacroProcessorState *state)
{
    if (!state->in_macro) {
        fprintf(stderr, "Error at line %d: 'mcroend' without matching 'mcro'\n",
                state->line_num);
        logMessage("ERROR line %d: mcroend without mcro start\n", state->line_num);
        state->is_error = 1;
        return 0;  /* Break */
    }
    
    state->in_macro = 0;
    
    if (!handle_mcro_end(state->line_num, state->name, state->macro_table,
                        state->macro_body)) {
        state->is_error = 1;
        free(state->macro_body);
        state->macro_body = NULL;
        return 0;  /* Break */
    }
    
    return 1;  /* Continue */
}

/*
 * Handle mcro definition start.
 * Returns: 1 to continue processing, 0 to break (error occurred)
 */
static int handleMcroStart(MacroProcessorState *state)
{
    if (!handle_mcro_start(state->line, state->line_num, state->name,
                          &state->macro_body, &state->macro_capacity,
                          &state->macro_length)) {
        state->is_error = 1;
        return 0;  /* Break */
    }
    state->in_macro = 1;
    return 1;  /* Continue */
}

/*
 * Handle a regular line or macro expansion.
 * Returns: 1 to continue, 0 to break (error), -1 for special case (inside macro)
 */
static int handleRegularLine(MacroProcessorState *state)
{
    MacroNode *macro = lookupMacro(state->macro_table, state->line);
    if (macro != NULL) {
        logMessage("Line %d: Expanding macro '%s'\n", state->line_num, state->line);
        fprintf(state->out_fp, "%s", macro->body);
    } else {
        fprintf(state->out_fp, "%s\n", state->line);
    }
    return 1;  /* Continue */
}

/*
 * Process a single line: determine its type and handle accordingly.
 * Returns: 1 to continue, 0 to break (fatal error)
 */
static int processLine(MacroProcessorState *state)
{
    char *first_word;
    
    /* Check for macro definition end */
    if (isMcroDefinitionEnd(state->line)) {
        return handleMcroEnd(state);
    }
    
    /* Check for macro definition start */
    first_word = get_first_word(state->line);
    if (first_word != NULL && strcmp(first_word, "mcro") == 0) {
        free(first_word);
        return handleMcroStart(state);
    }
    
    /* Inside a macro: collect lines */
    if (state->in_macro) {
        free(first_word);
        if (!appendToMacroBody(state, state->line)) {
            state->is_error = 1;
            free(state->macro_body);
            state->macro_body = NULL;
            return 0;  /* Break */
        }
        return 1;  /* Continue */
    }
    
    /* Regular line or macro call */
    free(first_word);
    return handleRegularLine(state);
}

/*
 * Finalize: close files, destroy table, cleanup partial output on error.
 */
static void finalizeResources(
    MacroProcessorState *state, const char *output_file, int had_errors)
{
    if (state->macro_table != NULL) {
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
    }
    
    if (had_errors && remove(output_file) == 0) {
        logMessage("Cleaned up partial output file: %s\n", output_file);
    }
}

int expandMacros(const char *input_file, const char *output_file)
{
    MacroProcessorState state;
    char *table_str;
    
    /* Initialize resources */
    if (initializeResources(input_file, output_file, &state) < 0) {
        return -1;  /* I/O failure */
    }
    
    logMessage("=== MACRO PROCESSOR START ===\n");
    logMessage("Input: %s\n", input_file);
    logMessage("Output: %s\n", output_file);
    
    /* Main processing loop */
    while (fgets(state.line, MAX_LINE_LENGTH + 1, state.in_fp) != NULL) {
        state.line_num++;
        
        /* Preprocess: remove newline, trim, remove comments */
        preprocessLine(state.line);
        
        /* Skip empty lines */
        if (strlen(state.line) == 0) {
            continue;
        }
        
        /* Process the line and check for continuance */
        if (!processLine(&state)) {
            break;  /* Fatal error, stop processing */
        }
    }
    
    /* Check for unclosed macro at EOF */
    if (state.in_macro) {
        fprintf(stderr, "Error: Unclosed macro '%s' - missing 'mcroend' at end of file\n",
                state.name);
        logMessage("ERROR: Unclosed macro at EOF - expecting mcroend for '%s'\n",
                   state.name);
        state.is_error = 1;
        if (state.macro_body != NULL) {
            free(state.macro_body);
            state.macro_body = NULL;
        }
    }
    
    /* Log final state  */
    if (state.is_error) {
        logMessage("=== MACRO PROCESSOR FAILED ===\n");
    } else {
        /* Log final macro table and completion info */
        table_str = macroTableToString(state.macro_table);
        logMessage("\n=== FINAL MACRO TABLE ===\n%s", table_str);
        free(table_str);
        logMessage("=== MACRO PROCESSOR COMPLETE ===\n");
        logMessage("Lines processed: %d\n", state.line_num);
    }
    
    /* Cleanup resources, delete output on error */
    finalizeResources(&state, output_file, state.is_error);
    
    return state.is_error;
}
