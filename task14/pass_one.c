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
 * @return 1 if valid, 0 otherwise (an error has been appended)
 */
static int validateSymbolToken(const char *token, int line_number, ErrorList *errors)
{
    if (strlen(token) > MAX_LABEL_LENGTH) {
        appendError(errors, line_number, "Symbol name exceeds maximum length");
        return 0;
    }
    if (!isValidSymbolName(token)) {
        appendError(errors, line_number, "Invalid symbol name syntax");
        return 0;
    }
    if (isReservedWord(token)) {
        appendError(errors, line_number, "Symbol name is a reserved word");
        return 0;
    }
    return 1;
}

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

    if (!validateSymbolToken(token, line_number, errors)) {
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

typedef enum {
    NUM_OK,
    NUM_MISSING,   /* no digits found; *p left unchanged */
    NUM_OVERFLOW   /* digits found, but the magnitude overflows long on this platform */
} NumberParseResult;

/**
 * Parses one signed decimal integer per the language spec (section 6):
 * optional '+'/'-' then decimal digits. Advances *p past what it consumed
 * on success; leaves *p unchanged on NUM_MISSING.
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
 * @return 1 if value fits the directive's storage width, 0 otherwise
 */
static int valueFitsDirective(Directive directive, long value)
{
    switch (directive) {
        case DIRECTIVE_DB: return value >= -128 && value <= 127;
        case DIRECTIVE_DH: return value >= -32768 && value <= 32767;
        case DIRECTIVE_DW: return value >= (-2147483647L - 1) && value <= 2147483647L;
        default:           return 0;
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
        appendError(errors, line_number, "Missing value list");
        out->has_error = 1;
        return;
    }

    for (;;) {
        result = parseSignedInteger(&p, &value);

        if (result == NUM_MISSING) {
            appendError(errors, line_number, "Expected a number");
            out->has_error = 1;
            return;
        }
        if (result == NUM_OVERFLOW || !valueFitsDirective(directive, value)) {
            snprintf(message, sizeof(message), "Value out of range for %s", directiveToString(directive));
            appendError(errors, line_number, message);
            out->has_error = 1;
            return;
        }
        if (appendDirectiveValue(out, value) != 0) {
            appendError(errors, line_number, "Out of memory");
            out->has_error = 1;
            return;
        }

        p = skipWhitespace(p);
        if (*p == '\0') {
            return; /* value list complete */
        }
        if (*p != ',') {
            appendError(errors, line_number, "Expected ',' between values");
            out->has_error = 1;
            return;
        }
        p = skipWhitespace(p + 1);
        if (*p == '\0' || *p == ',') {
            appendError(errors, line_number, "Expected a value after ','");
            out->has_error = 1;
            return;
        }
    }
}

/**
 * Parses a double-quoted string (.asciz) into out: one value per character,
 * plus a trailing 0 terminator, per the language spec.
 * @param rest the line, after the directive name
 * @param out the node to fill in
 * @param line_number for error reporting
 * @param errors error accumulator
 */
static void parseAsciz(const char *rest, DataDirectiveNode *out, int line_number, ErrorList *errors)
{
    const char *p = skipWhitespace(rest);

    if (*p != '"') {
        appendError(errors, line_number, "Expected '\"' to start .asciz string");
        out->has_error = 1;
        return;
    }
    p++;

    for (; *p != '"' && *p != '\0'; p++) {
        if (!isprint((unsigned char)*p)) {
            appendError(errors, line_number, "String contains a non-printable character");
            out->has_error = 1;
            return;
        }
        if (appendDirectiveValue(out, (long)(unsigned char)*p) != 0) {
            appendError(errors, line_number, "Out of memory");
            out->has_error = 1;
            return;
        }
    }

    if (*p != '"') {
        appendError(errors, line_number, "Missing closing '\"' for .asciz string");
        out->has_error = 1;
        return;
    }

    p = skipWhitespace(p + 1);
    if (*p != '\0') {
        appendError(errors, line_number, "Extra characters after .asciz string");
        out->has_error = 1;
        return;
    }

    if (appendDirectiveValue(out, 0) != 0) { /* trailing null terminator */
        appendError(errors, line_number, "Out of memory");
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
        appendError(errors, line_number, "Out of memory");
        return;
    }

    if (token[0] == '\0') {
        appendError(errors, line_number, "Missing symbol name");
    } else if (!validateSymbolToken(token, line_number, errors)) {
        /* validateSymbolToken already appended the error */
    } else if (*skipWhitespace(after) != '\0') {
        appendError(errors, line_number, "Extra characters after symbol name");
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

    rest = skipWhitespace(rest);

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
                out->directive = createDataDirectiveNode();
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

    out->operation = createOperationNode();
    if (out->operation == NULL) {
        appendError(errors, line_number, "Out of memory");
        return LINE_TYPE_BLANK;
    }
    parseOperationLine(operation, rest, out->operation, line_number, errors);
    return LINE_TYPE_OPERATION;
}

/**
 * Checks name against the macro table, appending an error if it collides
 * (spec: a symbol can't be both a macro name and a label/extern name).
 * Shared by insertLabelSymbol and handleExternLine -- the rest of their
 * insertion logic differs (see each one's own comment).
 * @param context the assembler context (macros, errors)
 * @param name the candidate symbol name
 * @param line_number for error reporting
 * @return 1 if name is free to use, 0 if it collided (an error was appended)
 */
static int rejectMacroNameCollision(AssemblerContext *context, const char *name, int line_number)
{
    if (lookupMacro(context->macros, name) != NULL) {
        appendError(context->errors, line_number, "Symbol name is also a macro name");
        return 0;
    }
    return 1;
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
    if (!label->has_label) {
        return;
    }

    if (!rejectMacroNameCollision(context, label->name, line_number)) {
        return;
    }

    if (insertSymbol(context->symbols, label->name, value, type) != 0) {
        appendError(context->errors, line_number, "Duplicate label");
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
        insertSymbol(context->symbols, symbol_name, 0, SYMBOL_TYPE_EXTERNAL);
    } else if (existing->type != SYMBOL_TYPE_EXTERNAL) {
        appendError(context->errors, line_number, "Symbol already defined with a different attribute");
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
                if (appendOperation(context->operations, payload.operation) != 0) {
                    free(payload.operation);
                    payload.operation = NULL;
                    appendError(context->errors, line_number, "Out of memory");
                }
                ic += 4; /* every instruction is 4 bytes, regardless of grammar errors */
                break;

            case LINE_TYPE_DATA_DIRECTIVE:
                insertLabelSymbol(context, &label, dc, SYMBOL_TYPE_DATA, line_number);
                payload.directive->address = dc;
                dc += payload.directive->value_count *
                      getDirectiveByteWidth(payload.directive->directive);
                if (appendDataDirective(context->directives, payload.directive) != 0) {
                    free(payload.directive->values);
                    free(payload.directive);
                    payload.directive = NULL;
                    appendError(context->errors, line_number, "Out of memory");
                }
                break;

            case LINE_TYPE_ENTRY:
                /* Label on this line is ignored per spec; the symbol itself
                 * may be a forward reference, so marking it waits for Pass Two. */
                appendEntry(context->entries, line_number, payload.symbol_name);
                break;

            case LINE_TYPE_EXTERN:
                /* Label on this line is ignored per spec. */
                handleExternLine(context, payload.symbol_name, line_number);
                break;
        }
    }

    fclose(fp);

    context->icf = ic;
    context->dcf = dc;
    relocateDataSymbols(context->symbols, ic);

    return 0;
}
