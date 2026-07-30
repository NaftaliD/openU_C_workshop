/*
 * assembler_core.c
 * Main entry point for the assembler.
 * Orchestrates: Macro processor (pre-assembler) -> (TODO: Pass 1) -> (TODO: Pass 2)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "utils.h"
#include "macro_processor.h"
#include "logger.h"

/**
 * Program entry point. Processes each given .as file independently through
 * the assembler pipeline (currently: macro expansion; Pass One/Two and
 * output-file generation are not wired in yet). An error in one file does
 * not stop the others from being processed.
 * @param argc number of command-line arguments (including program name)
 * @param argv argument vector: argv[1..] are .as file paths
 * @return 0 on completion, 1 if no input files were given
 */
int main(int argc, char *argv[])
{
    int i;
    int status;
    char am_filename[MAX_FILENAME_LENGTH];
    const char *as_filename;
    const char *dot_pos;
    int filename_len;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1.as> [file2.as] ...\n", argv[0]);
        return 1;
    }
    
    /* Initialize debug logger */
    initLogger(LOG_FILE_PATH);
    logMessage(LOG_DEBUG, "=== ASSEMBLER START ===\n");
    logMessage(LOG_DEBUG, "Processing %d input file(s)\n", argc - 1);

    /* Process each provided .as file */
    for (i = 1; i < argc; i++) {
        as_filename = argv[i];

        logMessage(LOG_INFO, "Processing %s...\n", as_filename);
        
        /* TODO: Validate that filename ends with .as extension */
        
        /* Generate .am filename (replace .as with .am) */
        dot_pos = strrchr(as_filename, '.');
        if (dot_pos != NULL) {
            filename_len = dot_pos - as_filename;
            strncpy(am_filename, as_filename, filename_len);
            am_filename[filename_len] = '\0';
            strcat(am_filename, ".am");
        } else {
            /* No extension, just append .am */
            strcpy(am_filename, as_filename);
            strcat(am_filename, ".am");
        }
        
        /* Step 1: Macro expansion (pre-assembler) */
        logMessage(LOG_INFO, "  [1] Running macro processor...\n");
        logMessage(LOG_DEBUG, "\nProcessing file: %s\n", as_filename);
        status = expandMacros(as_filename, am_filename);

        if (status > 0) {
            logMessage(LOG_ERROR, "Error: Macro processor failed for %s (found %d error(s))\n",
                       as_filename, status);
            logMessage(LOG_INFO, "  Skipping file due to errors.\n\n");
            continue;
        }

        if (status < 0) {
            logMessage(LOG_ERROR, "Error: Could not process %s (file I/O error)\n", as_filename);
            logMessage(LOG_INFO, "  Skipping file due to I/O error.\n\n");
            continue;
        }

        logMessage(LOG_INFO, "  Macro expansion successful. Output: %s\n", am_filename);

        /* TODO: Step 2: Pass One & Two */
        /* TODO: Step 3: File generation (.ob, .ent, .ext) */
    }

    logMessage(LOG_DEBUG, "=== ASSEMBLER COMPLETE ===\n");
    closeLogger();
    
    return 0;
}
