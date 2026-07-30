/*
 * logger.h
 * Simple logging utility for debug mode.
 * Handles file-based logging with formatted output.
 */

#ifndef LOGGER_H
#define LOGGER_H

/* DEBUG -> log file only. INFO -> log file + stdout. ERROR -> log file + stderr. */
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_ERROR
} LogLevel;

/**
 * Opens the debug log file. Call once at program start. Does nothing if
 * IS_DEBUG is 0.
 * @param filename path to the log file
 */
void initLogger(const char *filename);

/**
 * Writes a formatted message, like printf, routed by level (see LogLevel).
 * @param level where the message goes
 * @param format printf-style format string
 */
void logMessage(LogLevel level, const char *format, ...);

/**
 * Closes the debug log file. Call once at program exit.
 */
void closeLogger(void);

#endif /* LOGGER_H */
