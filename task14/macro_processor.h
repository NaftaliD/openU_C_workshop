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
    char line[MAX_LINE_LENGTH + 1];
    char *macro_body;
    int macro_capacity;
    int macro_length;
    int line_num;
    int is_error;               /* 1 if error occurred, 0 otherwise */
    int in_macro;
} MacroProcessorState;

/*
 * expandMacros
 * Reads the input .as file, processes macro definitions and expansions,
 * and writes the expanded code to the output .am file.
 *

 * Validations:
 * - Macro names cannot be reserved language words
 * - Macro names must follow label syntax (letter, then alphanumeric, <= MAX_MACRO_NAME_LENGTH chars)
 * - mcro and mcroend lines must have exact syntax (no extra words)
 *
 * Returns: error count (0 = success, >0 = number of errors encountered)
 */
int expandMacros(const char *input_file, const char *output_file);

/*
 * isMcroDefinitionStart
 * Validates "mcro <name>" line format.
 * Must have exactly 2 tokens (no extra words), and the name must fit in
 * name_out (MAX_MACRO_NAME_LENGTH characters, checked before copying).
 * Returns 1 if valid, 0 if invalid (fills name_out if valid).
 */
int isMcroDefinitionStart(const char *line, char *name_out);

/*
 * isMcroDefinitionEnd
 * Validates "mcroend" line.
 * Must be exactly the word "mcroend", no extra words.
 * Returns 1 if valid, 0 if invalid.
 */
int isMcroDefinitionEnd(const char *line);

#endif /* MACRO_PROCESSOR_H */
