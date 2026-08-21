/*
 * pass_two.c
 * Implementation of Pass Two: resolves labels and encodes every
 * operation, writing .ob/.ent/.ext directly as each line is ready.
 */

#include <stdio.h>
#include <stdarg.h>
#include "common.h"
#include "pass_two.h"

/* R-type bit-field layout */
#define OPCODE_SHIFT 26
#define R_RS_SHIFT 21
#define R_RT_SHIFT 16
#define R_RD_SHIFT 11
#define R_FUNCT_SHIFT 6

/* I-type bit-field layout - opcode/rs/rt shifts match R-type's */
#define I_RS_SHIFT 21
#define I_RT_SHIFT 16

/* J-type bit-field layout */
#define J_REG_SHIFT 25

#define FIELD_MASK_16BIT 0xFFFFUL
#define FIELD_MASK_25BIT 0x1FFFFFFUL

/**
 * Writes one "AAAA BB BB..." memory-content line. count may be
 * less than 4 for the last line of the data image.
 * @param fp the .ob file, already open
 * @param address the line's 4-digit decimal address
 * @param bytes the bytes to print, in order
 * @param count how many of bytes to print (1-4)
 */
static void writeObjectLine(FILE *fp, int address, const unsigned char *bytes, int count)
{
    int i;

    fprintf(fp, "%04d", address);
    for (i = 0; i < count; i++) {
        fprintf(fp, " %02X", bytes[i]);
    }
    fprintf(fp, "\n");
}

/**
 * Splits word into its 4 little-endian bytes and writes them as one .ob
 * line at node->address (every operation is exactly 4 bytes, always
 * address-aligned - byte-order rule: bits 0-7 first).
 * @param fp the .ob file, already open
 * @param node the operation this word was encoded from
 * @param word the fully-encoded 32-bit operation
 */
static void writeOperationLine(FILE *fp, const OperationNode *node, unsigned long word)
{
    unsigned char bytes[4];

    bytes[0] = (unsigned char)(word & 0xFFUL);
    bytes[1] = (unsigned char)((word >> 8) & 0xFFUL);
    bytes[2] = (unsigned char)((word >> 16) & 0xFFUL);
    bytes[3] = (unsigned char)((word >> 24) & 0xFFUL);
    writeObjectLine(fp, node->address, bytes, 4);
}

/**
 * Appends one printf-style line to filename, reopened fresh each call.
 * @param filename the .ent or .ext path to append to
 * @param line_number for error reporting if the file can't be opened
 * @param on_open_failure the message to report if fopen fails
 * @param errors error accumulator
 * @param format printf-style format for the line (no trailing newline)
 * @return TRUE on success, FALSE if filename couldn't be opened (an error
 *         has been appended)
 */
static Boolean appendOutputLine(const char *filename, int line_number,
                                 const char *on_open_failure, ErrorList *errors,
                                 const char *format, ...)
{
    FILE *fp;
    va_list args;

    fp = fopen(filename, "a");
    if (fp == NULL) {
        appendError(errors, line_number, on_open_failure);
        return FALSE;
    }

    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
    fprintf(fp, "\n");

    fclose(fp);
    return TRUE;
}

/**
 * Looks up name in the symbol table, appending ERR_UNDEFINED_SYMBOL if
 * missing.
 * @param symbols the symbol table
 * @param name the label name to resolve
 * @param line_number for error reporting
 * @param errors error accumulator
 * @return the matching node, or NULL if not found (error already appended)
 */
static SymbolNode* lookupOperandLabel(SymbolTable *symbols, const char *name,
                                       int line_number, ErrorList *errors)
{
    SymbolNode *symbol = lookupSymbol(symbols, name);
    if (symbol == NULL) {
        appendError(errors, line_number, ERR_UNDEFINED_SYMBOL);
    }
    return symbol;
}

/**
 * Encodes add/sub/and/or/nor (3-register) and move/mvhi/mvlo (2-register,
 * rt=0), distinguished by operand_count. Operand order matches field order.
 * @param node the operation to encode
 * @return the fully-encoded 32-bit operation
 */
static unsigned long encodeRType(const OperationNode *node)
{
    unsigned long opcode = (unsigned long)getOpcode(node->op);
    unsigned long funct = (unsigned long)getFunct(node->op);
    unsigned long rs = (unsigned long)node->operands[0].value.register_number;
    unsigned long rt;
    unsigned long rd;

    if (node->operand_count == 3) {
        rt = (unsigned long)node->operands[1].value.register_number;
        rd = (unsigned long)node->operands[2].value.register_number;
    } else {
        rt = 0;
        rd = (unsigned long)node->operands[1].value.register_number;
    }

    return (opcode << OPCODE_SHIFT) | (rs << R_RS_SHIFT) | (rt << R_RT_SHIFT) |
           (rd << R_RD_SHIFT) | (funct << R_FUNCT_SHIFT);
}

/**
 * Encodes addi/subi/andi/ori/nori and lb/sb/lw/sw/lh/sh. Syntax order is
 * rs, immed, rt but field order is rs, rt, immed - No funct field exists for I-type.
 * @param node the operation to encode
 * @return the fully-encoded 32-bit operation
 */
static unsigned long encodeITypeArithmetic(const OperationNode *node)
{
    unsigned long opcode = (unsigned long)getOpcode(node->op);
    unsigned long rs = (unsigned long)node->operands[0].value.register_number;
    unsigned long rt = (unsigned long)node->operands[2].value.register_number;
    long immed = node->operands[1].value.immediate_value;

    return (opcode << OPCODE_SHIFT) | (rs << I_RS_SHIFT) | (rt << I_RT_SHIFT) |
           ((unsigned long)immed & FIELD_MASK_16BIT);
}

/**
 * Encodes bne/beq/blt/bgt. rs/rt are field order; the label (operands[2])
 * must be internal, and its distance must fit in 16 bits.
 * @param node the operation to encode
 * @param symbols the symbol table
 * @param errors error accumulator
 * @param word_out receives the encoded operation on success
 * @return TRUE on success, FALSE otherwise (an error has been appended)
 */
static Boolean encodeITypeBranch(const OperationNode *node, SymbolTable *symbols,
                                  ErrorList *errors, unsigned long *word_out)
{
    unsigned long opcode = (unsigned long)getOpcode(node->op);
    unsigned long rs = (unsigned long)node->operands[0].value.register_number;
    unsigned long rt = (unsigned long)node->operands[1].value.register_number;
    const char *label_name = node->operands[2].value.label_name;
    SymbolNode *symbol;
    int immed;

    symbol = lookupOperandLabel(symbols, label_name, node->line_number, errors);
    if (symbol == NULL) {
        return FALSE;
    }
    if (symbol->type == SYMBOL_TYPE_EXTERNAL) {
        appendError(errors, node->line_number, ERR_BRANCH_EXTERNAL);
        return FALSE;
    }

    immed = symbol->value - node->address;
    if (immed < MIN_16BIT_SIGNED || immed > MAX_16BIT_SIGNED) {
        appendError(errors, node->line_number, ERR_BRANCH_OUT_OF_RANGE);
        return FALSE;
    }

    *word_out = (opcode << OPCODE_SHIFT) | (rs << I_RS_SHIFT) | (rt << I_RT_SHIFT) |
                ((unsigned long)immed & FIELD_MASK_16BIT);
    return TRUE;
}

/**
 * Resolves jmp/la/call's target: register form needs no lookup; label form
 * resolves via the symbol table, recording external usages to ext_filename.
 * @param node the operation whose first operand is the jmp/la/call target
 * @param symbols the symbol table
 * @param errors error accumulator
 * @param ext_filename the .ext path to append an external usage to, if needed
 * @param reg_out receives the reg bit
 * @param address_out receives the 25-bit address field
 * @return TRUE on success, FALSE otherwise (an error has been appended)
 */
static Boolean resolveJTypeTarget(const OperationNode *node, SymbolTable *symbols,
                                   ErrorList *errors, const char *ext_filename,
                                   unsigned long *reg_out, unsigned long *address_out)
{
    const Operand *target = &node->operands[0];
    SymbolNode *symbol;

    if (target->type == OPERAND_REGISTER) {
        *reg_out = 1;
        *address_out = (unsigned long)target->value.register_number;
        return TRUE;
    }

    *reg_out = 0;
    symbol = lookupOperandLabel(symbols, target->value.label_name, node->line_number, errors);
    if (symbol == NULL) {
        return FALSE;
    }

    if (symbol->type != SYMBOL_TYPE_EXTERNAL) {
        *address_out = (unsigned long)symbol->value;
        return TRUE;
    }

    *address_out = 0;
    return appendOutputLine(ext_filename, node->line_number, ERR_CANNOT_WRITE_EXT_FILE, errors,
                             "%s %04d", symbol->name, node->address);
}

/**
 * Encodes jmp/la/call via resolveJTypeTarget above.
 * @param node the operation to encode
 * @param symbols the symbol table
 * @param errors error accumulator
 * @param ext_filename the .ext path to append an external usage to, if needed
 * @param word_out receives the encoded operation on success
 * @return TRUE on success, FALSE otherwise (an error has been appended)
 */
static Boolean encodeJType(const OperationNode *node, SymbolTable *symbols,
                            ErrorList *errors, const char *ext_filename,
                            unsigned long *word_out)
{
    unsigned long opcode = (unsigned long)getOpcode(node->op);
    unsigned long reg;
    unsigned long address;

    if (!resolveJTypeTarget(node, symbols, errors, ext_filename, &reg, &address)) {
        return FALSE;
    }

    *word_out = (opcode << OPCODE_SHIFT) | (reg << J_REG_SHIFT) | (address & FIELD_MASK_25BIT);
    return TRUE;
}

/**
 * Dispatches one operation to its shape's encoder. Nodes reaching here are
 * always valid (Pass One already validated), so this never re-validates.
 * @param node the operation to encode
 * @param symbols the symbol table
 * @param errors error accumulator
 * @param ext_filename the .ext path to append an external usage to, if needed
 * @param word_out receives the encoded operation on success
 * @return TRUE on success, FALSE otherwise (an error has been appended)
 */
static Boolean encodeOperation(const OperationNode *node, SymbolTable *symbols,
                                ErrorList *errors, const char *ext_filename,
                                unsigned long *word_out)
{
    switch (getOperationType(node->op)) {
        case 'R':
            *word_out = encodeRType(node);
            return TRUE;
        case 'I':
            switch (node->op) {
                case OP_BNE:
                case OP_BEQ:
                case OP_BLT:
                case OP_BGT:
                    return encodeITypeBranch(node, symbols, errors, word_out);
                case OP_ADDI:
                case OP_SUBI:
                case OP_ANDI:
                case OP_ORI:
                case OP_NORI:
                case OP_LB:
                case OP_SB:
                case OP_LW:
                case OP_SW:
                case OP_LH:
                case OP_SH:
                    *word_out = encodeITypeArithmetic(node);
                    return TRUE;
                default:
                    return FALSE; /* unreachable: every I-type op is one of the above */
            }
        case 'J':
            switch (node->op) {
                case OP_HLT:
                    *word_out = (unsigned long)getOpcode(node->op) << OPCODE_SHIFT;
                    return TRUE;
                case OP_JMP:
                case OP_LA:
                case OP_CALL:
                    return encodeJType(node, symbols, errors, ext_filename, word_out);
                default:
                    return FALSE; /* unreachable: every J-type op is one of the above */
            }
        default:
            return FALSE; /* unreachable: every node has a valid, already-looked-up op */
    }
}

/**
 * Packs every value's bytes (little-endian) continuously into 4-byte .ob
 * lines, with no gaps at directive boundaries (unlike operations). Only
 * the first node's address is read; the rest is a running count.
 * @param fp the .ob file, already open
 * @param directives the data-directive list to pack
 */
static void writeDataDirectiveBytes(FILE *fp, const DataDirectiveList *directives)
{
    unsigned char pending[4];
    int pending_count = 0;
    int pending_start = 0;
    int address;
    int i, j, b;

    if (directives->count == 0) {
        return;
    }
    address = directives->items[0]->address;

    for (i = 0; i < directives->count; i++) {
        DataDirectiveNode *node = directives->items[i];
        int width = getDirectiveByteWidth(node->directive);

        for (j = 0; j < node->value_count; j++) {
            long value = node->values[j];

            for (b = 0; b < width; b++) {
                if (pending_count == 0) {
                    pending_start = address;
                }
                pending[pending_count++] = (unsigned char)((value >> (b * 8)) & 0xFF);
                address++;
                if (pending_count == 4) {
                    writeObjectLine(fp, pending_start, pending, 4);
                    pending_count = 0;
                }
            }
        }
    }

    if (pending_count > 0) {
        writeObjectLine(fp, pending_start, pending, pending_count);
    }
}

/**
 * Encodes and writes every operation in order; a failed encode is skipped
 * (already recorded as an error) rather than aborting the loop.
 * @param fp the .ob file, already open
 * @param operations the parsed operation list from Pass One
 * @param symbols the symbol table
 * @param errors error accumulator
 * @param ext_filename the .ext path to append an external usage to, if needed
 */
static void writeOperations(FILE *fp, const OperationList *operations, SymbolTable *symbols,
                             ErrorList *errors, const char *ext_filename)
{
    int i;
    unsigned long word;

    for (i = 0; i < operations->count; i++) {
        OperationNode *node = operations->items[i];
        if (encodeOperation(node, symbols, errors, ext_filename, &word)) {
            writeOperationLine(fp, node, word);
        }
    }
}

/**
 * Walks the entry list once: marks each symbol via markEntry, appending its
 * "SYMBOL_NAME VVVV" line to ent_filename on a fresh mark, or appending the
 * matching error.
 * @param entries the deferred .entry records from Pass One
 * @param symbols the symbol table
 * @param errors error accumulator
 * @param ent_filename the .ent path to append a line to, if needed
 */
static void writeEntries(const EntryList *entries, SymbolTable *symbols,
                          ErrorList *errors, const char *ent_filename)
{
    int i;

    for (i = 0; i < entries->count; i++) {
        EntryNode *entry = &entries->items[i];
        ResultCode result = markEntry(symbols, entry->symbol_name);

        switch (result) {
            case RC_SUCCESS: {
                SymbolNode *symbol = lookupSymbol(symbols, entry->symbol_name);
                appendOutputLine(ent_filename, entry->line_number, ERR_CANNOT_WRITE_ENT_FILE,
                                  errors, "%s %04d", symbol->name, symbol->value);
                break;
            }
            case RC_DUPLICATE:
                break; /* already written on the first occurrence */
            case RC_NOT_FOUND:
                appendError(errors, entry->line_number, ERR_UNDEFINED_SYMBOL);
                break;
            default: /* RC_CONFLICT: symbol exists but is external */
                appendError(errors, entry->line_number, ERR_SYMBOL_ATTRIBUTE_CONFLICT);
                break;
        }
    }
}

Boolean runPassTwo(const char *ob_filename, const char *ent_filename,
                    const char *ext_filename, AssemblerContext *context)
{
    FILE *ob_fp;

    /* Clear stale .ent/.ext from a previous run before appending fresh output. */
    remove(ent_filename);
    remove(ext_filename);

    ob_fp = fopen(ob_filename, "w");
    if (ob_fp == NULL) {
        return FALSE;
    }

    fprintf(ob_fp, "%d %d\n", context->icf - INITIAL_IC, context->dcf);

    writeOperations(ob_fp, context->operations, context->symbols, context->errors, ext_filename);
    writeDataDirectiveBytes(ob_fp, context->directives);
    writeEntries(context->entries, context->symbols, context->errors, ent_filename);

    fclose(ob_fp);

    if (errorListHasErrors(context->errors)) {
        remove(ob_filename);
        remove(ent_filename);
        remove(ext_filename);
    }

    return TRUE;
}
