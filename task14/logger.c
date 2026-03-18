/*
 * logger.c
 * Simple logging utility implementation.
 */

#include <stdio.h>
#include <stdarg.h>
#include "common.h"
#include "logger.h"

static FILE *log_file = NULL;

void Logger_init(const char *filename)
{
    if (!IS_DEBUG) {
        return;  /* Logging disabled in non-debug mode */
    }
    
    log_file = fopen(filename, "w");
    if (log_file == NULL) {
        fprintf(stderr, "Warning: Cannot open debug log file %s\n", filename);
        return;
    }
    
    Logger_log("[DEBUG LOG INITIALIZED]\n");
}

void Logger_log(const char *format, ...)
{
    va_list args;
    
    if (!IS_DEBUG || log_file == NULL) {
        return;
    }
    
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fflush(log_file);  /* Immediate flush for real-time logging */
}

void Logger_destroy(void)
{
    if (log_file != NULL) {
        Logger_log("[DEBUG LOG CLOSED]\n");
        fclose(log_file);
        log_file = NULL;
    }
}
