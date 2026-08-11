/*
 * assembler_core.c
 * Entry point and per-file pipeline orchestration.
 * Macro processor (pre-assembler) -> Pass One -> Pass Two (+ .ob/.ent/.ext)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "utils.h"
#include "macro_processor.h"
#include "pass_one.h"
#include "pass_two.h"
#include "assembler_context.h"
#include "logger.h"

/**
 * Derives an output path for as_filename: same base name, given extension
 * (if as_filename has no extension, extension is simply appended).
 * @param as_filename the source .as path
 * @param extension the extension to append, e.g. AM_FILE_EXTENSION
 * @param filename_out receives the derived path; must hold at least
 *        MAX_FILENAME_LENGTH bytes
 * @return TRUE on success, FALSE if as_filename is too long for
 *         filename_out (base name + extension + terminator must fit in
 *         MAX_FILENAME_LENGTH)
 */
static Boolean deriveOutputFilename(const char *as_filename, const char *extension,
                                     char *filename_out)
{
    const char *dot_pos = strrchr(as_filename, '.');
    int base_length = dot_pos != NULL ? (int)(dot_pos - as_filename) : (int)strlen(as_filename);

    if (base_length > MAX_FILENAME_LENGTH - (int)strlen(extension) - 1) {
        return FALSE;
    }

    strncpy(filename_out, as_filename, base_length);
    filename_out[base_length] = '\0';
    strcat(filename_out, extension);
    return TRUE;
}

/**
 * Dumps every Pass One result held in context to the debug log (file only)
 * -- a placeholder for real .ob/.ent/.ext generation, which needs Pass
 * Two's encoding to exist first.
 * @param context the populated assembler context
 */
static void dumpPassOneResults(const AssemblerContext *context)
{
    char *dump;

    /* Gated here, not just left to logMessage: building these means
     * walking every list with a malloc/snprintf/free per node, which isn't
     * worth paying for on every run just to have logMessage throw it away
     * when debug logging is off. */
    if (!IS_DEBUG) {
        return;
    }

    dump = symbolTableToString(context->symbols);
    logMessage(LOG_DEBUG, "%s", dump);
    free(dump);

    dump = operationListToString(context->operations);
    logMessage(LOG_DEBUG, "%s", dump);
    free(dump);

    dump = dataDirectiveListToString(context->directives);
    logMessage(LOG_DEBUG, "%s", dump);
    free(dump);

    dump = entryListToString(context->entries);
    logMessage(LOG_DEBUG, "%s", dump);
    free(dump);

    dump = errorListToString(context->errors);
    logMessage(LOG_DEBUG, "%s", dump);
    free(dump);
}

/**
 * Runs macro expansion, storing the resulting macro table in context on
 * success (Pass One needs it: a label can't share a name with a macro).
 * @param as_filename source .as path
 * @param am_filename destination .am path
 * @param context the assembler context to populate
 * @return TRUE to continue to Pass One, FALSE to skip this file
 */
static Boolean runMacroExpansionStep(const char *as_filename, const char *am_filename,
                                      AssemblerContext *context)
{
    ResultCode status;

    logMessage(LOG_INFO, "  [1] Running macro processor...\n");
    status = expandMacros(as_filename, am_filename, &context->macros);

    switch (status) {
        case RC_SUCCESS:
            break;
        case RC_INVALID:
            logMessage(LOG_ERROR, ERR_MACRO_PROCESSOR_FAILED, as_filename);
            return FALSE;
        case RC_OUT_OF_MEMORY:
            logMessage(LOG_ERROR, ERR_FILE_OUT_OF_MEMORY, as_filename);
            return FALSE;
        default: /* RC_IO_ERROR */
            logMessage(LOG_ERROR, ERR_MACRO_PROCESSOR_IO_FAILED, as_filename);
            return FALSE;
    }

    logMessage(LOG_INFO, "  Macro expansion successful. Output: %s\n", am_filename);
    return TRUE;
}

/**
 * Runs Pass One and logs the outcome.
 * @param as_filename source .as path, for error messages
 * @param am_filename expanded .am path to read
 * @param context the assembler context to populate
 * @return TRUE if Pass One completed with no errors, FALSE otherwise
 */
static Boolean runPassOneStep(const char *as_filename, const char *am_filename,
                               AssemblerContext *context)
{
    logMessage(LOG_INFO, "  [2] Running Pass One...\n");

    if (runPassOne(am_filename, context) == FALSE) {
        logMessage(LOG_ERROR, ERR_PASS_ONE_OPEN_FAILED, am_filename);
        return FALSE;
    }

    logMessage(LOG_INFO, "  Pass One done: IC=%d, DC=%d, %d error(s)\n",
               context->icf, context->dcf, context->errors->count);
    dumpPassOneResults(context);

    if (errorListHasErrors(context->errors)) {
        logMessage(LOG_ERROR, ERR_PASS_ONE_FAILED, as_filename, context->errors->count);
        return FALSE;
    }

    return TRUE;
}

/**
 * Runs Pass Two and logs the outcome.
 * @param as_filename source .as path, for error messages
 * @param ob_filename path to write the object file to
 * @param ent_filename path to write entry symbols to, if any exist
 * @param ext_filename path to write external symbol usages to, if any exist
 * @param context the assembler context, fully populated by Pass One
 * @return TRUE if Pass Two completed with no errors, FALSE otherwise
 */
static Boolean runPassTwoStep(const char *as_filename, const char *ob_filename,
                               const char *ent_filename, const char *ext_filename,
                               AssemblerContext *context)
{
    logMessage(LOG_INFO, "  [3] Running Pass Two...\n");

    if (runPassTwo(ob_filename, ent_filename, ext_filename, context) == FALSE) {
        logMessage(LOG_ERROR, ERR_PASS_TWO_OPEN_FAILED, ob_filename);
        return FALSE;
    }

    if (errorListHasErrors(context->errors)) {
        logMessage(LOG_ERROR, ERR_PASS_TWO_FAILED, as_filename, context->errors->count);
        return FALSE;
    }

    logMessage(LOG_INFO, "  Pass Two done. Output: %s\n", ob_filename);
    return TRUE;
}

/**
 * @param filename a file path
 * @return TRUE if filename ends with AS_FILE_EXTENSION, FALSE otherwise
 */
static Boolean hasAsExtension(const char *filename)
{
    size_t ext_len = sizeof(AS_FILE_EXTENSION) - 1; /* excludes the null terminator */
    size_t length = strlen(filename);
    return length >= ext_len && strcmp(filename + length - ext_len, AS_FILE_EXTENSION) == 0;
}

/**
 * Runs the full pipeline for one .as file. Owns the AssemblerContext for
 * the whole pipeline (not just one step), since later steps consume what
 * earlier ones build -- e.g. Pass Two will need everything Pass One built.
 * @param as_filename the .as file to process
 * @return TRUE if every step succeeded, FALSE otherwise
 */
static Boolean processFile(const char *as_filename)
{
    char am_filename[MAX_FILENAME_LENGTH];
    char ob_filename[MAX_FILENAME_LENGTH];
    char ent_filename[MAX_FILENAME_LENGTH];
    char ext_filename[MAX_FILENAME_LENGTH];
    AssemblerContext *context;
    Boolean rc;

    if (!hasAsExtension(as_filename)) {
        logMessage(LOG_ERROR, ERR_MISSING_AS_EXTENSION, as_filename);
        return FALSE;
    }

    context = createAssemblerContext();
    if (context == NULL) {
        logMessage(LOG_ERROR, ERR_FILE_OUT_OF_MEMORY, as_filename);
        return FALSE;
    }

    if (!deriveOutputFilename(as_filename, AM_FILE_EXTENSION, am_filename) ||
        !deriveOutputFilename(as_filename, OB_FILE_EXTENSION, ob_filename) ||
        !deriveOutputFilename(as_filename, ENT_FILE_EXTENSION, ent_filename) ||
        !deriveOutputFilename(as_filename, EXT_FILE_EXTENSION, ext_filename)) {
        logMessage(LOG_ERROR, ERR_FILENAME_TOO_LONG, as_filename);
        destroyAssemblerContext(context);
        return FALSE;
    }

    logMessage(LOG_INFO, "Processing %s...\n", as_filename);
    logMessage(LOG_DEBUG, "\nProcessing file: %s\n", as_filename);

    rc = runMacroExpansionStep(as_filename, am_filename, context) &&
         runPassOneStep(as_filename, am_filename, context) &&
         runPassTwoStep(as_filename, ob_filename, ent_filename, ext_filename, context);

    destroyAssemblerContext(context);
    return rc;
}

/**
 * Program entry point. Processes each given .as file independently through
 * the full assembler pipeline (macro expansion, Pass One, Pass Two). An
 * error in one file does not stop the others from being processed.
 * @param argc number of command-line arguments (including program name)
 * @param argv argument vector: argv[1..] are .as file paths
 * @return 0 on completion, 1 if no input files were given
 */
int main(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        fprintf(stderr, USAGE_MESSAGE, argv[0]);
        return 1;
    }

    initLogger(LOG_FILE_PATH);
    logMessage(LOG_DEBUG, "=== ASSEMBLER START ===\n");
    logMessage(LOG_DEBUG, "Processing %d input file(s)\n", argc - 1);

    for (i = 1; i < argc; i++) {
        processFile(argv[i]);
    }

    logMessage(LOG_DEBUG, "=== ASSEMBLER COMPLETE ===\n");
    closeLogger();

    return 0;
}
