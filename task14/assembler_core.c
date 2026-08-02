/*
 * assembler_core.c
 * Entry point and per-file pipeline orchestration.
 * Macro processor (pre-assembler) -> Pass One -> (TODO: Pass Two)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "utils.h"
#include "macro_processor.h"
#include "pass_one.h"
#include "assembler_context.h"
#include "logger.h"

/**
 * Derives the .am path for as_filename: same base name, .am extension (if
 * as_filename has no extension, .am is simply appended).
 * @param as_filename the source .as path
 * @param am_filename_out receives the derived path; must hold at least
 *        MAX_FILENAME_LENGTH bytes
 * @return TRUE on success, FALSE if as_filename is too long for
 *         am_filename_out (base name + AM_FILE_EXTENSION + terminator must
 *         fit in MAX_FILENAME_LENGTH)
 */
static Boolean deriveAmFilename(const char *as_filename, char *am_filename_out)
{
    const char *dot_pos = strrchr(as_filename, '.');
    int base_length = dot_pos != NULL ? (int)(dot_pos - as_filename) : (int)strlen(as_filename);

    if (base_length > MAX_FILENAME_LENGTH - (int)sizeof(AM_FILE_EXTENSION)) {
        return FALSE;
    }

    strncpy(am_filename_out, as_filename, base_length);
    am_filename_out[base_length] = '\0';
    strcat(am_filename_out, AM_FILE_EXTENSION);
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
    int status;

    logMessage(LOG_INFO, "  [1] Running macro processor...\n");
    status = expandMacros(as_filename, am_filename, &context->macros);

    if (status > 0) {
        logMessage(LOG_ERROR, ERR_MACRO_PROCESSOR_FAILED, as_filename);
        return FALSE;
    }
    if (status < 0) {
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

    if (runPassOne(am_filename, context) < 0) {
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
 */
static void processFile(const char *as_filename)
{
    char am_filename[MAX_FILENAME_LENGTH];
    AssemblerContext *context;

    if (!hasAsExtension(as_filename)) {
        logMessage(LOG_ERROR, ERR_MISSING_AS_EXTENSION, as_filename);
        return;
    }

    context = createAssemblerContext();
    if (context == NULL) {
        logMessage(LOG_ERROR, ERR_FILE_OUT_OF_MEMORY, as_filename);
        return;
    }

    if (!deriveAmFilename(as_filename, am_filename)) {
        logMessage(LOG_ERROR, ERR_FILENAME_TOO_LONG, as_filename);
        destroyAssemblerContext(context);
        return;
    }

    logMessage(LOG_INFO, "Processing %s...\n", as_filename);
    logMessage(LOG_DEBUG, "\nProcessing file: %s\n", as_filename);

    if (runMacroExpansionStep(as_filename, am_filename, context) &&
        runPassOneStep(as_filename, am_filename, context)) {
        /* TODO: Pass Two + file generation (.ob/.ent/.ext), once Pass
         * One's grammar is real. */
    }

    destroyAssemblerContext(context);
}

/**
 * Program entry point. Processes each given .as file independently through
 * the assembler pipeline (currently: macro expansion, then Pass One; Pass
 * Two and output-file generation are not wired in yet). An error in one
 * file does not stop the others from being processed.
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
