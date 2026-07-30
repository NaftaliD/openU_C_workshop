/*
 * pass_one.c
 * extractLabel and parseLine are real; the three per-line-type sub-parsers
 * are stubs (grammar not designed yet) so the overall wiring -- allocation,
 * ownership, dispatch -- can be exercised before that design happens.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "utils.h"
#include "pass_one.h"

int extractLabel(const char *line, LabelInfo *label_out, const char **rest_out,
                  int line_number, ErrorList *errors)
{
    char *token;
    const char *after;

    label_out->has_label = 0;
    label_out->name[0] = '\0';

    /* A label candidate ends at ':'; ending at whitespace/end-of-line first
     * means there's no label (getFirstToken doesn't touch line either way). */
    token = getFirstToken(line, ": \t", &after);
    if (token == NULL || *after != ':') {
        free(token);
        *rest_out = line;
        return 0;
    }

    *rest_out = after + 1;

    if (strlen(token) > MAX_LABEL_LENGTH) {
        appendError(errors, line_number, "Label name exceeds maximum length");
        free(token);
        return -1;
    }

    if (!isValidSymbolName(token)) {
        appendError(errors, line_number, "Invalid label syntax");
        free(token);
        return -1;
    }

    if (isReservedWord(token)) {
        appendError(errors, line_number, "Label name is a reserved word");
        free(token);
        return -1;
    }

    label_out->has_label = 1;
    strcpy(label_out->name, token);
    free(token);
    return 1;
}

/**
 * Parses the operand portion of an operation line into out.
 * TODO: operand count/type validation, register/immediate range checks.
 * Currently a stub: marks every line as an error so nothing downstream
 * trusts unparsed data.
 * @param op which operation this line uses (already looked up, never invalid)
 * @param rest the line, after the label and opcode name
 * @param out the node to fill in
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseOperationLine(Operation op, const char *rest, OperationNode *out,
                                int line_number, ErrorList *errors)
{
    out->line_number = line_number;
    out->op = op;
    out->operand_count = 0;
    out->address = ADDRESS_UNASSIGNED;
    out->has_error = 1;
    appendError(errors, line_number, "Operation grammar not yet implemented");
    (void)rest;
}

/**
 * Parses the operand portion of a data-directive line into out.
 * TODO: value list parsing (.db/.dw/.dh), string parsing (.asciz), and
 * per-directive range checks. Currently a stub.
 * @param directive which data directive this line uses
 * @param rest the line, after the label and directive name
 * @param out the node to fill in
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseDataDirectiveLine(Directive directive, const char *rest,
                                    DataDirectiveNode *out, int line_number,
                                    ErrorList *errors)
{
    out->line_number = line_number;
    out->directive = directive;
    out->values = NULL;
    out->value_count = 0;
    out->value_capacity = 0;
    out->address = ADDRESS_UNASSIGNED;
    out->has_error = 1;
    appendError(errors, line_number, "Directive grammar not yet implemented");
    (void)rest;
}

/**
 * Parses the single symbol-name operand of an .entry/.extern line into
 * symbol_name_out.
 * TODO: validate the symbol name. Currently a stub.
 * @param rest the line, after the directive name
 * @param symbol_name_out receives the symbol name
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseSymbolOperand(const char *rest, char *symbol_name_out,
                                int line_number, ErrorList *errors)
{
    symbol_name_out[0] = '\0';
    appendError(errors, line_number, "Symbol operand grammar not yet implemented");
    (void)rest;
}

LineType parseLine(const char *raw_line, int line_number,
                    ParsedLinePayload *out, LabelInfo *out_label,
                    ErrorList *errors)
{
    const char *rest;
    char *first_word;
    Directive directive;
    Operation operation;

    extractLabel(raw_line, out_label, &rest, line_number, errors);

    while (*rest == ' ' || *rest == '\t') {
        rest++;
    }

    if (*rest == '\0') {
        return LINE_TYPE_BLANK;
    }

    /* rest is known non-empty and free of leading whitespace at this point,
     * so getFirstToken (same delimiter set) is guaranteed to return a
     * non-empty token -- no need to check for one. */
    first_word = getFirstToken(rest, " \t", &rest);
    if (first_word == NULL) {
        appendError(errors, line_number, "Out of memory");
        return LINE_TYPE_BLANK;
    }

    while (*rest == ' ' || *rest == '\t') {
        rest++;
    }

    /* Per the language spec, a leading '.' is what makes a line a directive
     * -- not whether the name that follows is recognized. That has to be
     * checked before lookup, not derived from it, so a garbled ".foo" is
     * reported as an unknown DIRECTIVE, not an unknown operation. */
    if (first_word[0] == '.') {
        directive = directiveFromString(first_word);
        free(first_word);

        switch (directive) {
            case DIRECTIVE_ENTRY:
                parseSymbolOperand(rest, out->symbol_name, line_number, errors);
                return LINE_TYPE_ENTRY;

            case DIRECTIVE_EXTERN:
                parseSymbolOperand(rest, out->symbol_name, line_number, errors);
                return LINE_TYPE_EXTERN;

            case DIRECTIVE_INVALID:
                appendError(errors, line_number, "Unknown directive");
                return LINE_TYPE_BLANK;

            default: /* DIRECTIVE_DB / DIRECTIVE_DW / DIRECTIVE_DH / DIRECTIVE_ASCIZ */
                out->directive = (DataDirectiveNode *)malloc(sizeof(DataDirectiveNode));
                if (out->directive == NULL) {
                    appendError(errors, line_number, "Out of memory");
                    return LINE_TYPE_BLANK;
                }
                parseDataDirectiveLine(directive, rest, out->directive, line_number, errors);
                return LINE_TYPE_DATA_DIRECTIVE;
        }
    }

    /* Not '.'-prefixed, so per spec this is an operation attempt regardless
     * of whether the name turns out to be recognized -- same reasoning as
     * the directive branch above. */
    operation = operationFromString(first_word);
    free(first_word);

    if (operation == OP_INVALID) {
        appendError(errors, line_number, "Unknown operation");
        return LINE_TYPE_BLANK;
    }

    out->operation = (OperationNode *)malloc(sizeof(OperationNode));
    if (out->operation == NULL) {
        appendError(errors, line_number, "Out of memory");
        return LINE_TYPE_BLANK;
    }
    parseOperationLine(operation, rest, out->operation, line_number, errors);
    return LINE_TYPE_OPERATION;
}
