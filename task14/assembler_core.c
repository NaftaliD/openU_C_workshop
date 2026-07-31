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
 */
static void deriveAmFilename(const char *as_filename, char *am_filename_out)
{
    const char *dot_pos = strrchr(as_filename, '.');
    int base_length = dot_pos != NULL ? (int)(dot_pos - as_filename) : (int)strlen(as_filename);

    strncpy(am_filename_out, as_filename, base_length);
    am_filename_out[base_length] = '\0';
    strcat(am_filename_out, ".am");
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
 * @return 1 to continue to Pass One, 0 to skip this file
 */
static int runMacroExpansionStep(const char *as_filename, const char *am_filename,
                                  AssemblerContext *context)
{
    int status;

    logMessage(LOG_INFO, "  [1] Running macro processor...\n");
    status = expandMacros(as_filename, am_filename, &context->macros);

    if (status > 0) {
        logMessage(LOG_ERROR, "Error: Macro processor failed for %s (found %d error(s))\n",
                   as_filename, status);
        return 0;
    }
    if (status < 0) {
        logMessage(LOG_ERROR, "Error: Could not process %s (file I/O error)\n", as_filename);
        return 0;
    }

    logMessage(LOG_INFO, "  Macro expansion successful. Output: %s\n", am_filename);
    return 1;
}

/**
 * Runs Pass One and logs the outcome.
 * @param as_filename source .as path, for error messages
 * @param am_filename expanded .am path to read
 * @param context the assembler context to populate
 * @return 1 if Pass One completed with no errors, 0 otherwise
 */
static int runPassOneStep(const char *as_filename, const char *am_filename,
                           AssemblerContext *context)
{
    logMessage(LOG_INFO, "  [2] Running Pass One...\n");

    if (runPassOne(am_filename, context) < 0) {
        logMessage(LOG_ERROR, "Error: Could not open %s for Pass One\n", am_filename);
        return 0;
    }

    logMessage(LOG_INFO, "  Pass One done: IC=%d, DC=%d, %d error(s)\n",
               context->icf, context->dcf, context->errors->count);
    dumpPassOneResults(context);

    if (errorListHasErrors(context->errors)) {
        logMessage(LOG_ERROR, "Error: Pass One failed for %s (%d error(s))\n",
                   as_filename, context->errors->count);
        return 0;
    }

    return 1;
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
    AssemblerContext *context = createAssemblerContext();

    if (context == NULL) {
        logMessage(LOG_ERROR, "Error: Out of memory processing %s\n", as_filename);
        return;
    }

    deriveAmFilename(as_filename, am_filename);
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
        fprintf(stderr, "Usage: %s <file1.as> [file2.as] ...\n", argv[0]);
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
