/*
 * logger.h
 * Simple logging utility for debug mode.
 * Handles file-based logging with formatted output.
 */

#ifndef LOGGER_H
#define LOGGER_H

/*
 * initLogger
 * Initialize the debug logger with a file.
 * Should be called once at program start.
 * If debug mode is disabled, does nothing.
 */
void initLogger(const char *filename);

/*
 * logMessage
 * Write a formatted message to the debug log (debug mode only).
 * Similar to printf - supports format strings and arguments.
 */
void logMessage(const char *format, ...);

/*
 * closeLogger
 * Close and clean up the debug logger.
 * Should be called at program exit.
 */
void closeLogger(void);

#endif /* LOGGER_H */
