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
 * and writes the result to output_file. Validates that macro names aren't
 * reserved words, follow label syntax, and that mcro/mcroend lines have no
 * extra words.
 * @param input_file path to the source .as file
 * @param output_file path to write the expanded .am file
 * @param macros_out on success, receives the macro table (caller owns
 *                    destroying it via destroyMacroTable -- Pass One needs
 *                    it to reject a label reusing a macro name); NULL on
 *                    any failure
 * @return 0 on success, otherwise the number of errors found
 */
int expandMacros(const char *input_file, const char *output_file, MacroTable **macros_out);

/**
 * Validates a "mcro <name>" line: exactly two tokens, name short enough to
 * fit name_out.
 * @param line the line to validate
 * @param name_out receives the macro name if the line is valid
 * @return 1 if valid, 0 otherwise
 */
int isMcroDefinitionStart(const char *line, char *name_out);

/**
 * Validates a "mcroend" line: exactly the word "mcroend", nothing else.
 * @param line the line to validate
 * @return 1 if valid, 0 otherwise
 */
int isMcroDefinitionEnd(const char *line);

#endif /* MACRO_PROCESSOR_H */
