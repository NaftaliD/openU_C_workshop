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
    Logger_init(LOG_FILE_PATH);
    Logger_log("=== ASSEMBLER START ===\n");
    Logger_log("Processing %d input file(s)\n", argc - 1);
    
    /* Process each provided .as file */
    for (i = 1; i < argc; i++) {
        as_filename = argv[i];
        
        printf("Processing %s...\n", as_filename);
        
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
        printf("  [1] Running macro processor...\n");
        Logger_log("\nProcessing file: %s\n", as_filename);
        status = MacroProcessor_expand(as_filename, am_filename);
        
        if (status > 0) {
            fprintf(stderr, "Error: Macro processor failed for %s (found %d error(s))\n", 
                    as_filename, status);
            Logger_log("FATAL: Macro processor failed with %d error(s)\n", status);
            printf("  Skipping file due to errors.\n\n");
            continue;
        }
        
        if (status < 0) {
            fprintf(stderr, "Error: Could not process %s (file I/O error)\n", as_filename);
            Logger_log("FATAL: I/O error while processing %s\n", as_filename);
            printf("  Skipping file due to I/O error.\n\n");
            continue;
        }
        
        printf("  Macro expansion successful. Output: %s\n", am_filename);
        
        /* TODO: Step 2: Pass One & Two */
        /* TODO: Step 3: File generation (.ob, .ent, .ext) */
    }
    
    Logger_log("=== ASSEMBLER COMPLETE ===\n");
    Logger_destroy();
    
    return 0;
}
