/*
 * macro_processor.h
 * Macro processor (pre-assembler) header.
 * Declares the main preprocessing function and macro-specific validators.
 */

#ifndef MACRO_PROCESSOR_H
#define MACRO_PROCESSOR_H

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
    char name[31];              /* MAX_MACRO_NAME_LENGTH */
    char line[81];              /* MAX_LINE_LENGTH + 1 */
    char *macro_body;
    int macro_capacity;
    int macro_length;
    int line_num;
    int is_error;               /* 1 if error occurred, 0 otherwise */
    int in_macro;
} MacroProcessorState;

/*
 * MacroProcessor_expand
 * Reads the input .as file, processes macro definitions and expansions,
 * and writes the expanded code to the output .am file.
 * 

 * Validations:
 * - Macro names cannot be reserved language words
 * - mcro and mcroend lines must have exact syntax (no extra words)
 * 
 * Returns: error count (0 = success, >0 = number of errors encountered)
 */
int MacroProcessor_expand(const char *input_file, const char *output_file);

/*
 * MacroProcessor_isMcroDefinitionStart
 * Validates "mcro <name>" line format.
 * Must have exactly 2 tokens (no extra words).
 * Returns 1 if valid, 0 if invalid (fills name_out if valid).
 */
int MacroProcessor_isMcroDefinitionStart(const char *line, char *name_out);

/*
 * MacroProcessor_isMcroDefinitionEnd
 * Validates "mcroend" line.
 * Must be exactly the word "mcroend", no extra words.
 * Returns 1 if valid, 0 if invalid.
 */
int MacroProcessor_isMcroDefinitionEnd(const char *line);

#endif /* MACRO_PROCESSOR_H */
