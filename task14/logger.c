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
        fprintf(stderr, WARN_LOG_FILE_OPEN_FAILED, filename);
        return;
    }

    logMessage(LOG_DEBUG, "[DEBUG LOG INITIALIZED]\n");
}

void logMessage(LogLevel level, const char *format, ...)
{
    va_list args;

    if (IS_DEBUG && log_file != NULL) {
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fflush(log_file);  /* Immediate flush for real-time logging */
    }

    if (level == LOG_INFO) {
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    } else if (level == LOG_ERROR) {
        fprintf(stdout, "ERROR: ");
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
    }
}

void closeLogger(void)
{
    if (log_file != NULL) {
        logMessage(LOG_DEBUG, "[DEBUG LOG CLOSED]\n");
        fclose(log_file);
        log_file = NULL;
    }
}
