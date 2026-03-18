/*
 * logger.h
 * Simple logging utility for debug mode.
 * Handles file-based logging with formatted output.
 */

#ifndef LOGGER_H
#define LOGGER_H

/*
 * Logger_init
 * Initialize the debug logger with a file.
 * Should be called once at program start.
 * If debug mode is disabled, does nothing.
 */
void Logger_init(const char *filename);

/*
 * Logger_log
 * Write a formatted message to the debug log (debug mode only).
 * Similar to printf - supports format strings and arguments.
 */
void Logger_log(const char *format, ...);

/*
 * Logger_destroy
 * Close and clean up the debug logger.
 * Should be called at program exit.
 */
void Logger_destroy(void);

#endif /* LOGGER_H */
