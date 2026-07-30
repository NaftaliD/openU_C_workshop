/*
 * pass_one.h
 * Shared per-line types for Pass One, and the single entry point that turns
 * one preprocessed .am line into a typed record. The per-line-type grammar
 * (operand validation, register/immediate range checks, directive value
 * parsing) is still a stub -- see the TODOs in pass_one.c -- but the shape
 * and wiring are in place.
 */

#ifndef PASS_ONE_H
#define PASS_ONE_H

#include "common.h"
#include "operation_line.h"
#include "data_directive_line.h"
#include "error_list.h"

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
 * no space before it). Reuses isValidSymbolName/isReservedWord from utils.h.
 * @param line the line to scan
 * @param label_out receives has_label and the label name if one was found
 * @param rest_out receives a pointer to the remainder of line after the label
 * @param line_number for error reporting
 * @param errors error accumulator for invalid label syntax
 * @return 1 if a label was consumed, 0 if no label prefix was present, -1 if
 *         an invalid label prefix was present (an error was appended, but
 *         *rest_out still lets parsing continue)
 */
int extractLabel(const char *line, LabelInfo *label_out, const char **rest_out,
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

#endif /* PASS_ONE_H */
