/*
 * pass_one.c
 * Implementation of Pass One: label/line parsing and the driver loop.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "utils.h"
#include "pass_one.h"

/**
 * Validates a symbol-name token already extracted from the line: length,
 * character syntax, and not a reserved word.
 * @param token the candidate symbol name
 * @param line_number for error reporting
 * @param errors error accumulator
 * @return TRUE if valid, FALSE otherwise (an error has been appended)
 */
static Boolean validateSymbolToken(const char *token, int line_number, ErrorList *errors)
{
    if (strlen(token) > MAX_LABEL_LENGTH) {
        appendError(errors, line_number, ERR_SYMBOL_TOO_LONG);
        return FALSE;
    }
    if (!isValidSymbolName(token)) {
        appendError(errors, line_number, ERR_SYMBOL_INVALID_SYNTAX);
        return FALSE;
    }
    if (isReservedWord(token)) {
        appendError(errors, line_number, ERR_SYMBOL_RESERVED_WORD);
        return FALSE;
    }
    return TRUE;
}

Boolean extractLabel(const char *line, LabelInfo *label_out, const char **rest_out,
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
        return FALSE;
    }

    *rest_out = after + 1;

    if (!validateSymbolToken(token, line_number, errors)) {
        free(token);
        return FALSE;
    }

    label_out->has_label = 1;
    strcpy(label_out->name, token);
    free(token);
    return TRUE;
}

typedef enum {
    NUM_OK,
    NUM_MISSING,   /* no digits found; *p left unchanged */
    NUM_OVERFLOW   /* digits found, but the magnitude overflows long on this platform */
} NumberParseResult;

/**
 * Parses one signed decimal integer: optional '+'/'-' then decimal digits.
 * Advances *p past what it consumed on success; leaves *p unchanged on
 * NUM_MISSING.
 * @param p scan position, in/out
 * @param value_out receives the parsed value on NUM_OK
 * @return NUM_OK, NUM_MISSING, or NUM_OVERFLOW
 */
static NumberParseResult parseSignedInteger(const char **p, long *value_out)
{
    char *end;
    long value;

    errno = 0;
    value = strtol(*p, &end, 10);

    if (end == *p) {
        return NUM_MISSING; /* strtol leaves endptr == nptr when nothing valid was found */
    }
    if (errno == ERANGE) {
        *p = end;
        return NUM_OVERFLOW;
    }

    *p = end;
    *value_out = value;
    return NUM_OK;
}

/**
 * @param directive a numeric data directive (.db/.dw/.dh)
 * @param value a value already known to fit in a long
 * @return TRUE if value fits the directive's storage width, FALSE otherwise
 */
static Boolean valueFitsDirective(Directive directive, long value)
{
    switch (directive) {
        case DIRECTIVE_DB: return value >= MIN_8BIT_SIGNED && value <= MAX_8BIT_SIGNED;
        case DIRECTIVE_DH: return value >= MIN_16BIT_SIGNED && value <= MAX_16BIT_SIGNED;
        case DIRECTIVE_DW: return value >= MIN_32BIT_SIGNED && value <= MAX_32BIT_SIGNED;
        default:           return FALSE;
    }
}

/**
 * Parses a comma-separated list of signed integers (.db/.dw/.dh) into out.
 * Stops at the first malformed token; values already parsed are kept.
 * @param directive which numeric directive this is, for range checks
 * @param rest the line, after the directive name
 * @param out the node to fill in (values/value_count/value_capacity/has_error)
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseNumericValues(Directive directive, const char *rest, DataDirectiveNode *out,
                                int line_number, ErrorList *errors)
{
    const char *p = skipWhitespace(rest);
    long value;
    NumberParseResult result;
    char message[MAX_ERROR_MESSAGE_LENGTH];

    if (*p == '\0') {
        appendError(errors, line_number, ERR_VALUE_LIST_MISSING);
        out->has_error = 1;
        return;
    }

    for (;;) {
        result = parseSignedInteger(&p, &value);

        if (result == NUM_MISSING) {
            appendError(errors, line_number, ERR_EXPECTED_NUMBER);
            out->has_error = 1;
            return;
        }
        if (result == NUM_OVERFLOW || !valueFitsDirective(directive, value)) {
            snprintf(message, sizeof(message), ERR_VALUE_OUT_OF_RANGE, directiveToString(directive));
            appendError(errors, line_number, message);
            out->has_error = 1;
            return;
        }
        if (appendDirectiveValue(out, value) == FALSE) {
            appendError(errors, line_number, ERR_OUT_OF_MEMORY);
            out->has_error = 1;
            return;
        }

        p = skipWhitespace(p);
        if (*p == '\0') {
            return; /* value list complete */
        }
        if (*p != ',') {
            appendError(errors, line_number, ERR_EXPECTED_COMMA_VALUES);
            out->has_error = 1;
            return;
        }
        p = skipWhitespace(p + 1);
        if (*p == '\0' || *p == ',') {
            appendError(errors, line_number, ERR_EXPECTED_VALUE_AFTER_COMMA);
            out->has_error = 1;
            return;
        }
    }
}

/**
 * Parses a double-quoted string (.asciz) into out: one value per character,
 * plus a trailing 0 terminator.
 * @param rest the line, after the directive name
 * @param out the node to fill in
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseAsciz(const char *rest, DataDirectiveNode *out, int line_number, ErrorList *errors)
{
    const char *p = skipWhitespace(rest);

    if (*p != '"') {
        appendError(errors, line_number, ERR_ASCIZ_MISSING_OPEN_QUOTE);
        out->has_error = 1;
        return;
    }
    p++;

    for (; *p != '"' && *p != '\0'; p++) {
        if (!isprint((unsigned char)*p)) {
            appendError(errors, line_number, ERR_ASCIZ_NON_PRINTABLE);
            out->has_error = 1;
            return;
        }
        if (appendDirectiveValue(out, (long)(unsigned char)*p) == FALSE) {
            appendError(errors, line_number, ERR_OUT_OF_MEMORY);
            out->has_error = 1;
            return;
        }
    }

    if (*p != '"') {
        appendError(errors, line_number, ERR_ASCIZ_MISSING_CLOSE_QUOTE);
        out->has_error = 1;
        return;
    }

    p = skipWhitespace(p + 1);
    if (*p != '\0') {
        appendError(errors, line_number, ERR_ASCIZ_TRAILING_CHARS);
        out->has_error = 1;
        return;
    }

    if (appendDirectiveValue(out, 0) == FALSE) { /* trailing null terminator */
        appendError(errors, line_number, ERR_OUT_OF_MEMORY);
        out->has_error = 1;
    }
}

/**
 * Parses the operand portion of a data-directive line into out: a value
 * list for .db/.dw/.dh, or a quoted string for .asciz.
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
    out->has_error = 0;

    switch (directive) {
        case DIRECTIVE_DB:
        case DIRECTIVE_DW:
        case DIRECTIVE_DH:
            parseNumericValues(directive, rest, out, line_number, errors);
            break;
        case DIRECTIVE_ASCIZ:
            parseAsciz(rest, out, line_number, errors);
            break;
        case DIRECTIVE_ENTRY:
        case DIRECTIVE_EXTERN:
        case DIRECTIVE_INVALID:
            break; /* unreachable: parseLine only calls this for DB/DW/DH/ASCIZ */
    }
}

/**
 * Consumes one register operand ("$N"): validates the '$' prefix, then
 * parses a signed integer for N (accepting a leading sign here so "$-1"
 * reports as out-of-range rather than a syntax error) and range-checks it
 * against 0-31.
 * @param p scan position, in/out
 * @param register_out receives the register number on success
 * @param line_number for error reporting
 * @param errors error accumulator
 * @return TRUE on success, FALSE otherwise (an error has been appended)
 */
static Boolean parseRegisterOperand(const char **p, int *register_out, int line_number, ErrorList *errors)
{
    long value;
    NumberParseResult result;

    if (**p != '$') {
        appendError(errors, line_number, ERR_EXPECTED_REGISTER);
        return FALSE;
    }
    (*p)++;

    result = parseSignedInteger(p, &value);
    if (result == NUM_MISSING) {
        appendError(errors, line_number, ERR_EXPECTED_REGISTER);
        return FALSE;
    }
    if (result == NUM_OVERFLOW || value < MIN_REGISTER_NUMBER || value > MAX_REGISTER_NUMBER) {
        appendError(errors, line_number, ERR_REGISTER_OUT_OF_RANGE);
        return FALSE;
    }

    *register_out = (int)value;
    return TRUE;
}

/**
 * Consumes one immediate operand: a signed decimal integer within the
 * 16-bit range shared by every I-type immediate field.
 * @param p scan position, in/out
 * @param value_out receives the value on success
 * @param line_number for error reporting
 * @param errors error accumulator
 * @return TRUE on success, FALSE otherwise (an error has been appended)
 */
static Boolean parseImmediateOperand(const char **p, long *value_out, int line_number, ErrorList *errors)
{
    NumberParseResult result = parseSignedInteger(p, value_out);

    if (result == NUM_MISSING) {
        appendError(errors, line_number, ERR_EXPECTED_NUMBER);
        return FALSE;
    }
    if (result == NUM_OVERFLOW || *value_out < MIN_16BIT_SIGNED || *value_out > MAX_16BIT_SIGNED) {
        appendError(errors, line_number, ERR_IMMEDIATE_OUT_OF_RANGE);
        return FALSE;
    }
    return TRUE;
}

/**
 * Consumes one label-reference operand: a token ending at whitespace or ','
 * (comma included so trailing punctuation isn't swallowed into the name),
 * validated via validateSymbolToken.
 * @param p scan position, in/out
 * @param label_out receives the name on success (must hold MAX_LABEL_LENGTH+1)
 * @param line_number for error reporting
 * @param errors error accumulator
 * @return TRUE on success, FALSE otherwise (an error has been appended)
 */
static Boolean parseLabelRefOperand(const char **p, char *label_out, int line_number, ErrorList *errors)
{
    char *token;
    Boolean valid;

    token = getFirstToken(*p, " \t,", p);
    if (token == NULL) {
        appendError(errors, line_number, ERR_OUT_OF_MEMORY);
        return FALSE;
    }

    valid = validateSymbolToken(token, line_number, errors);
    if (valid) {
        strcpy(label_out, token);
    }
    free(token);
    return valid;
}

/**
 * Requires a ',' between operands (optional whitespace on both sides).
 * @param p scan position, in/out
 * @param line_number for error reporting
 * @param errors error accumulator
 * @return TRUE on success, FALSE otherwise (an error has been appended)
 */
static Boolean expectOperandSeparator(const char **p, int line_number, ErrorList *errors)
{
    *p = skipWhitespace(*p);
    if (**p != ',') {
        appendError(errors, line_number, ERR_EXPECTED_COMMA_OPERANDS);
        return FALSE;
    }
    *p = skipWhitespace(*p + 1);
    return TRUE;
}

/**
 * Requires nothing but whitespace remains at p.
 * @param p scan position (not advanced further)
 * @param line_number for error reporting
 * @param errors error accumulator
 * @return TRUE if the line ends here, FALSE otherwise (an error has been appended)
 */
static Boolean expectEndOfOperands(const char *p, int line_number, ErrorList *errors)
{
    if (*skipWhitespace(p) != '\0') {
        appendError(errors, line_number, ERR_TRAILING_OPERAND_CHARS);
        return FALSE;
    }
    return TRUE;
}

/**
 * Parses `count` comma-separated register operands: "$rs, $rt[, $rd]".
 * Shared by the 3-register shape (add/sub/and/or/nor) and the 2-register
 * shape (move/mvhi/mvlo) -- identical grammar, only the count differs.
 * @param count how many register operands this operation takes (2 or 3)
 * @param rest the line, after the opcode name
 * @param out the node to fill in (operands/operand_count/has_error)
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseRegisterOperands(int count, const char *rest, OperationNode *out,
                                   int line_number, ErrorList *errors)
{
    const char *p = skipWhitespace(rest);
    int i;
    int register_number;

    for (i = 0; i < count; i++) {
        if (i > 0 && !expectOperandSeparator(&p, line_number, errors)) {
            out->has_error = 1;
            return;
        }
        if (!parseRegisterOperand(&p, &register_number, line_number, errors)) {
            out->has_error = 1;
            return;
        }
        out->operands[i].type = OPERAND_REGISTER;
        out->operands[i].value.register_number = register_number;
        out->operand_count++;
    }

    if (!expectEndOfOperands(p, line_number, errors)) {
        out->has_error = 1;
    }
}

/**
 * Parses "$rs, immed, $rt" -- shared by I-type arithmetic
 * (addi/subi/andi/ori/nori) and I-type memory (lb/sb/lw/sw/lh/sh), which
 * have identical operand grammar even though their semantics differ.
 * @param rest the line, after the opcode name
 * @param out the node to fill in
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseRegImmedRegOperands(const char *rest, OperationNode *out,
                                      int line_number, ErrorList *errors)
{
    const char *p = skipWhitespace(rest);
    int rs;
    long immediate;
    int rt;

    if (!parseRegisterOperand(&p, &rs, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    out->operands[0].type = OPERAND_REGISTER;
    out->operands[0].value.register_number = rs;
    out->operand_count++;

    if (!expectOperandSeparator(&p, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    if (!parseImmediateOperand(&p, &immediate, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    out->operands[1].type = OPERAND_IMMEDIATE;
    out->operands[1].value.immediate_value = immediate;
    out->operand_count++;

    if (!expectOperandSeparator(&p, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    if (!parseRegisterOperand(&p, &rt, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    out->operands[2].type = OPERAND_REGISTER;
    out->operands[2].value.register_number = rt;
    out->operand_count++;

    if (!expectEndOfOperands(p, line_number, errors)) {
        out->has_error = 1;
    }
}

/**
 * Parses "$rs, $rt, label" (bne/beq/blt/bgt).
 * @param rest the line, after the opcode name
 * @param out the node to fill in
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseRegRegLabelOperands(const char *rest, OperationNode *out,
                                      int line_number, ErrorList *errors)
{
    const char *p = skipWhitespace(rest);
    int rs;
    int rt;

    if (!parseRegisterOperand(&p, &rs, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    out->operands[0].type = OPERAND_REGISTER;
    out->operands[0].value.register_number = rs;
    out->operand_count++;

    if (!expectOperandSeparator(&p, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    if (!parseRegisterOperand(&p, &rt, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    out->operands[1].type = OPERAND_REGISTER;
    out->operands[1].value.register_number = rt;
    out->operand_count++;

    if (!expectOperandSeparator(&p, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    out->operands[2].type = OPERAND_LABEL_REF;
    if (!parseLabelRefOperand(&p, out->operands[2].value.label_name, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    out->operand_count++;

    if (!expectEndOfOperands(p, line_number, errors)) {
        out->has_error = 1;
    }
}

/**
 * Parses a single label operand (la/call).
 * @param rest the line, after the opcode name
 * @param out the node to fill in
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseLabelOnlyOperand(const char *rest, OperationNode *out,
                                   int line_number, ErrorList *errors)
{
    const char *p = skipWhitespace(rest);

    out->operands[0].type = OPERAND_LABEL_REF;
    if (!parseLabelRefOperand(&p, out->operands[0].value.label_name, line_number, errors)) {
        out->has_error = 1;
        return;
    }
    out->operand_count++;

    if (!expectEndOfOperands(p, line_number, errors)) {
        out->has_error = 1;
    }
}

/**
 * Parses jmp's one operand, which is either a label ("jmp label") or a
 * register ("jmp $N") -- disambiguated by whether it starts with '$'.
 * @param rest the line, after the opcode name
 * @param out the node to fill in
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseJmpOperand(const char *rest, OperationNode *out, int line_number, ErrorList *errors)
{
    const char *p = skipWhitespace(rest);
    int register_number;

    if (*p == '$') {
        if (!parseRegisterOperand(&p, &register_number, line_number, errors)) {
            out->has_error = 1;
            return;
        }
        out->operands[0].type = OPERAND_REGISTER;
        out->operands[0].value.register_number = register_number;
    } else {
        out->operands[0].type = OPERAND_LABEL_REF;
        if (!parseLabelRefOperand(&p, out->operands[0].value.label_name, line_number, errors)) {
            out->has_error = 1;
            return;
        }
    }
    out->operand_count++;

    if (!expectEndOfOperands(p, line_number, errors)) {
        out->has_error = 1;
    }
}

/**
 * Parses the operand portion of an operation line into out, dispatched by
 * operand *shape* (not just R/I/J class -- e.g. R-type splits into
 * 3-register and 2-register shapes; I-type into reg-immed-reg and
 * reg-reg-label shapes; J-type into label-only, jmp's dual form, and none).
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
    out->has_error = 0;

    switch (op) {
        case OP_ADD: case OP_SUB: case OP_AND: case OP_OR: case OP_NOR:
            parseRegisterOperands(3, rest, out, line_number, errors);
            break;

        case OP_MOVE: case OP_MVHI: case OP_MVLO:
            parseRegisterOperands(2, rest, out, line_number, errors);
            break;

        case OP_ADDI: case OP_SUBI: case OP_ANDI: case OP_ORI: case OP_NORI:
        case OP_LB: case OP_SB: case OP_LW: case OP_SW: case OP_LH: case OP_SH:
            parseRegImmedRegOperands(rest, out, line_number, errors);
            break;

        case OP_BNE: case OP_BEQ: case OP_BLT: case OP_BGT:
            parseRegRegLabelOperands(rest, out, line_number, errors);
            break;

        case OP_LA: case OP_CALL:
            parseLabelOnlyOperand(rest, out, line_number, errors);
            break;

        case OP_JMP:
            parseJmpOperand(rest, out, line_number, errors);
            break;

        case OP_HLT:
            if (!expectEndOfOperands(rest, line_number, errors)) {
                out->has_error = 1;
            }
            break;

        case OP_INVALID:
            break; /* unreachable: parseLine only calls this for a looked-up operation */
    }
}

/**
 * Parses the single symbol-name operand of an .entry/.extern line into
 * symbol_name_out: one token, following label-name syntax, nothing after it.
 * @param rest the line, after the directive name
 * @param symbol_name_out receives the symbol name
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseSymbolOperand(const char *rest, char *symbol_name_out,
                                int line_number, ErrorList *errors)
{
    char *token;
    const char *after;

    symbol_name_out[0] = '\0';

    token = getFirstToken(rest, " \t", &after);
    if (token == NULL) {
        appendError(errors, line_number, ERR_OUT_OF_MEMORY);
        return;
    }

    if (token[0] == '\0') {
        appendError(errors, line_number, ERR_SYMBOL_MISSING);
    } else if (!validateSymbolToken(token, line_number, errors)) {
        /* validateSymbolToken already appended the error */
    } else if (*skipWhitespace(after) != '\0') {
        appendError(errors, line_number, ERR_SYMBOL_TRAILING_CHARS);
    } else {
        strcpy(symbol_name_out, token);
    }

    free(token);
}

LineType parseLine(const char *raw_line, int line_number,
                    ParsedLinePayload *out, LabelInfo *out_label,
                    ErrorList *errors)
{
    const char *rest;
    char *first_word;
    Directive directive;
    Operation operation;

    /* Return value unused: the error's already in errors, which gates output. */
    extractLabel(raw_line, out_label, &rest, line_number, errors);

    rest = skipWhitespace(rest);

    if (*rest == '\0') {
        if (out_label->has_label) {
            appendError(errors, line_number, ERR_LABEL_WITHOUT_STATEMENT);
        }
        return LINE_TYPE_BLANK;
    }

    /* rest is known non-empty and free of leading whitespace at this point,
     * so getFirstToken (same delimiter set) is guaranteed to return a
     * non-empty token -- no need to check for one. */
    first_word = getFirstToken(rest, " \t", &rest);
    if (first_word == NULL) {
        appendError(errors, line_number, ERR_OUT_OF_MEMORY);
        return LINE_TYPE_BLANK;
    }

    rest = skipWhitespace(rest);

    /* A leading '.' is what makes a line a directive -- not whether the
     * name that follows is recognized. That has to be checked before
     * lookup, not derived from it, so a garbled ".foo" is reported as an
     * unknown DIRECTIVE, not an unknown operation. */
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
                appendError(errors, line_number, ERR_UNKNOWN_DIRECTIVE);
                return LINE_TYPE_BLANK;

            default: /* DIRECTIVE_DB / DIRECTIVE_DW / DIRECTIVE_DH / DIRECTIVE_ASCIZ */
                out->directive = createDataDirectiveNode();
                if (out->directive == NULL) {
                    appendError(errors, line_number, ERR_OUT_OF_MEMORY);
                    return LINE_TYPE_BLANK;
                }
                parseDataDirectiveLine(directive, rest, out->directive, line_number, errors);
                return LINE_TYPE_DATA_DIRECTIVE;
        }
    }

    /* Not '.'-prefixed, so this is an operation attempt regardless of
     * whether the name turns out to be recognized -- same reasoning as the
     * directive branch above. */
    operation = operationFromString(first_word);
    free(first_word);

    if (operation == OP_INVALID) {
        appendError(errors, line_number, ERR_UNKNOWN_OPERATION);
        return LINE_TYPE_BLANK;
    }

    out->operation = createOperationNode();
    if (out->operation == NULL) {
        appendError(errors, line_number, ERR_OUT_OF_MEMORY);
        return LINE_TYPE_BLANK;
    }
    parseOperationLine(operation, rest, out->operation, line_number, errors);
    return LINE_TYPE_OPERATION;
}

/**
 * Checks name against the macro table, appending an error if it collides.
 * Shared by insertLabelSymbol and handleExternLine -- the rest of their
 * insertion logic differs (see each one's own comment).
 * @param context the assembler context (macros, errors)
 * @param name the candidate symbol name
 * @param line_number for error reporting
 * @return TRUE if name is free to use, FALSE if it collided (an error was appended)
 */
static Boolean rejectMacroNameCollision(AssemblerContext *context, const char *name, int line_number)
{
    if (lookupMacro(context->macros, name) != NULL) {
        appendError(context->errors, line_number, ERR_SYMBOL_IS_MACRO_NAME);
        return FALSE;
    }
    return TRUE;
}

/**
 * Registers the label extracted from an operation/data-directive line as a
 * symbol, if the line actually had one -- a no-op otherwise. A failure
 * (beyond a macro-name collision) can only mean the name was already
 * defined as a label (extractLabel already rejected bad syntax and
 * reserved words) -- unlike .extern, a label may never repeat.
 * @param context the assembler context (macros, symbols, errors)
 * @param label the label extracted from this line
 * @param value the address to store (current IC or DC)
 * @param type SYMBOL_TYPE_CODE or SYMBOL_TYPE_DATA
 * @param line_number for error reporting
 */
static void insertLabelSymbol(AssemblerContext *context, const LabelInfo *label, int value,
                               SymbolType type, int line_number)
{
    ResultCode result;

    if (!label->has_label) {
        return;
    }

    if (!rejectMacroNameCollision(context, label->name, line_number)) {
        return;
    }

    if ((result = insertSymbol(context->symbols, label->name, value, type)) != RC_SUCCESS) {
        appendError(context->errors, line_number,
                    result == RC_DUPLICATE ? ERR_DUPLICATE_LABEL : ERR_OUT_OF_MEMORY);
    }
}

/**
 * Handles one .extern line: inserts the symbol as external unless it's
 * already known. A repeat .extern of the same name is allowed (no-op),
 * unlike a label; a name already defined with a different attribute is an
 * error.
 * @param context the assembler context (macros, symbols, errors)
 * @param symbol_name the externed name
 * @param line_number for error reporting
 */
static void handleExternLine(AssemblerContext *context, const char *symbol_name, int line_number)
{
    SymbolNode *existing;

    if (!rejectMacroNameCollision(context, symbol_name, line_number)) {
        return;
    }

    existing = lookupSymbol(context->symbols, symbol_name);

    if (existing == NULL) {
        /* Only RC_OUT_OF_MEMORY is reachable here  */
        if (insertSymbol(context->symbols, symbol_name, 0, SYMBOL_TYPE_EXTERNAL) != RC_SUCCESS) {
            appendError(context->errors, line_number, ERR_OUT_OF_MEMORY);
        }
    } else if (existing->type != SYMBOL_TYPE_EXTERNAL) {
        appendError(context->errors, line_number, ERR_SYMBOL_ATTRIBUTE_CONFLICT);
    }
}

int runPassOne(const char *am_file, AssemblerContext *context)
{
    FILE *fp;
    char line[LINE_BUFFER_SIZE];
    int line_number = 0;
    int ic = INITIAL_IC;
    int dc = INITIAL_DC;
    ParsedLinePayload payload;
    LabelInfo label;
    LineType type;

    fp = fopen(am_file, "r");
    if (fp == NULL) {
        return -1;
    }

    /* .am lines already had comments/blank lines removed and were trimmed
     * by the pre-assembler -- only fgets's trailing newline needs stripping. */
    while (fgets(line, LINE_BUFFER_SIZE, fp) != NULL) {
        line_number++;
        trimWhitespace(line);
        if (line[0] == '\0') {
            continue;
        }

        type = parseLine(line, line_number, &payload, &label, context->errors);

        switch (type) {
            case LINE_TYPE_BLANK:
                break;

            case LINE_TYPE_OPERATION:
                insertLabelSymbol(context, &label, ic, SYMBOL_TYPE_CODE, line_number);
                payload.operation->address = ic;
                if (appendOperation(context->operations, payload.operation) == FALSE) {
                    free(payload.operation);
                    payload.operation = NULL;
                    appendError(context->errors, line_number, ERR_OUT_OF_MEMORY);
                }
                ic += INSTRUCTION_SIZE_BYTES; /* regardless of grammar errors */
                break;

            case LINE_TYPE_DATA_DIRECTIVE:
                insertLabelSymbol(context, &label, dc, SYMBOL_TYPE_DATA, line_number);
                payload.directive->address = dc;
                dc += payload.directive->value_count *
                      getDirectiveByteWidth(payload.directive->directive);
                if (appendDataDirective(context->directives, payload.directive) == FALSE) {
                    free(payload.directive->values);
                    free(payload.directive);
                    payload.directive = NULL;
                    appendError(context->errors, line_number, ERR_OUT_OF_MEMORY);
                }
                break;

            case LINE_TYPE_ENTRY:
                /* Label on this line is ignored; the symbol itself may be a
                 * forward reference, so marking it waits for Pass Two. An
                 * empty name means parseSymbolOperand already failed and
                 * appended its own error -- nothing valid to record here. */
                if (payload.symbol_name[0] != '\0' &&
                    appendEntry(context->entries, line_number, payload.symbol_name) == FALSE) {
                    appendError(context->errors, line_number, ERR_OUT_OF_MEMORY);
                }
                break;

            case LINE_TYPE_EXTERN:
                /* Label on this line is ignored. Same empty-name guard as
                 * LINE_TYPE_ENTRY above. */
                if (payload.symbol_name[0] != '\0') {
                    handleExternLine(context, payload.symbol_name, line_number);
                }
                break;
        }
    }

    fclose(fp);

    context->icf = ic;
    context->dcf = dc;
    relocateDataSymbols(context->symbols, ic);
    relocateDataDirectiveAddresses(context->directives, ic);

    return 0;
}
