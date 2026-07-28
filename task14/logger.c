/*
 * logger.c
 * Simple logging utility implementation.
 */

#include <stdio.h>
#include <stdarg.h>
#include "common.h"
#include "logger.h"

static FILE *log_file = NULL;

void initLogger(const char *filename)
{
    if (!IS_DEBUG) {
        return;  /* Logging disabled in non-debug mode */
    }

    log_file = fopen(filename, "w");
    if (log_file == NULL) {
        fprintf(stderr, "Warning: Cannot open debug log file %s\n", filename);
        return;
    }

    logMessage("[DEBUG LOG INITIALIZED]\n");
}

void logMessage(const char *format, ...)
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

void closeLogger(void)
{
    if (log_file != NULL) {
        logMessage("[DEBUG LOG CLOSED]\n");
        fclose(log_file);
        log_file = NULL;
    }
}
