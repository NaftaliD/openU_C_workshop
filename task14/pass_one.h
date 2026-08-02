/*
 * pass_one.h
 * Per-line types for Pass One, the parser entry point, and the driver loop.
 */

#ifndef PASS_ONE_H
#define PASS_ONE_H

#include "common.h"
#include "operation_line.h"
#include "data_directive_line.h"
#include "entry_list.h"
#include "error_list.h"
#include "symbol_table.h"
#include "assembler_context.h"

/* Result of extracting an optional leading "name:" label. Transient: the
 * driver inserts it into the symbol table and discards it -- never stored
 * on OperationNode/DataDirectiveNode. */
typedef struct {
    int  has_label;
    char name[MAX_LABEL_LENGTH + 1];
} LabelInfo;

typedef enum {
    LINE_TYPE_OPERATION,
    LINE_TYPE_DATA_DIRECTIVE,
    LINE_TYPE_ENTRY,
    LINE_TYPE_EXTERN,
    LINE_TYPE_BLANK
} LineType;

/* Exactly one arm is meaningful, selected by the returned LineType. ENTRY
 * and EXTERN share the plain string arm -- neither allocates. */
typedef union {
    OperationNode     *operation;
    DataDirectiveNode *directive;
    char               symbol_name[MAX_LABEL_LENGTH + 1];
} ParsedLinePayload;

/**
 * Detects and consumes a leading "name:" label (colon immediately adjacent,
 * no space before it).
 * @param line the line to scan
 * @param label_out receives has_label and the label name if one was found
 * @param rest_out receives a pointer to the remainder of line after the label
 * @param line_number for error reporting
 * @param errors error accumulator for invalid label syntax
 * @return TRUE if a label was found and consumed, FALSE otherwise (no label
 *         prefix, or an invalid one -- *rest_out still lets parsing continue
 *         either way, and an error was appended for the invalid case)
 */
Boolean extractLabel(const char *line, LabelInfo *label_out, const char **rest_out,
                      int line_number, ErrorList *errors);

/**
 * Single entry point: classifies one line and parses it into out, selecting
 * the matching ParsedLinePayload arm. Pure with respect to file-wide state --
 * does not touch IC/DC or the symbol table; that's the driver's job.
 * @param raw_line one already-preprocessed .am line
 * @param line_number for error reporting
 * @param out receives the parsed operation/directive/symbol-name
 * @param out_label receives the line's label, if any
 * @param errors error accumulator
 * @return which arm of out was filled in
 */
LineType parseLine(const char *raw_line, int line_number,
                    ParsedLinePayload *out, LabelInfo *out_label,
                    ErrorList *errors);

/**
 * Runs Pass One over an already-expanded .am file into context: IC/DC
 * bookkeeping, symbol/label insertion, and node lists in file order.
 * context->macros must already be set (see expandMacros); the rest of
 * context must already be allocated (see createAssemblerContext).
 * @param am_file path to the expanded .am file to read
 * @param context the assembler context to populate
 * @return 0 on success, -1 if am_file could not be opened
 */
int runPassOne(const char *am_file, AssemblerContext *context);

#endif /* PASS_ONE_H */
