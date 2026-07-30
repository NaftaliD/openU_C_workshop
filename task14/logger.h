/*
 * logger.h
 * Simple logging utility for debug mode.
 * Handles file-based logging with formatted output.
 */

#ifndef LOGGER_H
#define LOGGER_H

/**
 * Opens the debug log file. Call once at program start. Does nothing if
 * IS_DEBUG is 0.
 * @param filename path to the log file
 */
void initLogger(const char *filename);

/**
 * Writes a formatted message to the debug log (debug mode only), like printf.
 * @param format printf-style format string
 */
void logMessage(const char *format, ...);

/**
 * Closes the debug log file. Call once at program exit.
 */
void closeLogger(void);

#endif /* LOGGER_H */
