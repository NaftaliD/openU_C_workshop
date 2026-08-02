/*
 * macro_processor.h
 * Macro processor (pre-assembler) header.
 * Declares the main preprocessing function and macro-specific validators.
 */

#ifndef MACRO_PROCESSOR_H
#define MACRO_PROCESSOR_H

#include "common.h"
#include "macro_table.h"

/*
 * MacroProcessorState
 * Encapsulates all state needed during macro processing.
 * Used by helper functions to share context without parameter explosion.
 */
typedef struct {
    FILE *in_fp;
    FILE *out_fp;
    MacroTable *macro_table;
    char name[MAX_MACRO_NAME_LENGTH + 1];
    char line[LINE_BUFFER_SIZE];
    char *macro_body;
    int macro_capacity;
    int macro_length;
    int line_num;
    int is_error;               /* 1 if error occurred, 0 otherwise */
    int in_macro;
} MacroProcessorState;

/**
 * Reads input_file, expands all mcro/mcroend definitions and invocations,
 * and writes the result to output_file. Stops at the first error rather
 * than collecting every one (see README.md for why).
 * @param input_file path to the source .as file
 * @param output_file path to write the expanded .am file
 * @param macros_out on success, receives the macro table (caller owns
 *                    destroying it via destroyMacroTable -- Pass One needs
 *                    it to reject a label reusing a macro name); NULL on
 *                    any failure
 * @return 0 on success, 1 if an error was found, -1 on I/O failure
 */
int expandMacros(const char *input_file, const char *output_file, MacroTable **macros_out);

/**
 * Validates a "mcro <name>" line: exactly two tokens, name short enough to
 * fit name_out.
 * @param line the line to validate
 * @param name_out receives the macro name if the line is valid
 * @return TRUE if valid, FALSE otherwise
 */
Boolean isMcroDefinitionStart(const char *line, char *name_out);

/**
 * Validates a "mcroend" line: exactly the word "mcroend", nothing else.
 * @param line the line to validate
 * @return TRUE if valid, FALSE otherwise
 */
Boolean isMcroDefinitionEnd(const char *line);

#endif /* MACRO_PROCESSOR_H */
