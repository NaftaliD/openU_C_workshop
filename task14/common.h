/*
 * common.h
 * Common definitions and constants for the assembler.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Set to 1 to enable debug logging and verbose output */
#define IS_DEBUG 1
#define LOG_FILE_PATH "tests/assembler.log"


#define MAX_LINE_LENGTH 80
#define MAX_MACRO_NAME_LENGTH 31
#define MAX_FILENAME_LENGTH 256

/* The read buffer needs 2 bytes of headroom beyond MAX_LINE_LENGTH so a
 * line exactly at the limit can be told apart from one over it  */
#define LINE_BUFFER_SIZE (MAX_LINE_LENGTH + 2)

#define MAX_LABEL_LENGTH 31       /* distinct from MAX_MACRO_NAME_LENGTH though equal */
#define MAX_OPERANDS 3
#define ADDRESS_UNASSIGNED (-1)   /* 0 is a valid DC value, so can't mean "unset" */
#define MAX_ERROR_MESSAGE_LENGTH 128
#define INITIAL_LIST_CAPACITY 16  /* growable-array lists start here, double on growth */
#define INITIAL_IC 100            /* first instruction is loaded at address 100 */
#define INITIAL_DC 0

#define MIN_REGISTER_NUMBER 0
#define MAX_REGISTER_NUMBER 31

/* Signed storage-width ranges, shared by data-directive values (.db/.dh/.dw)
 * and instruction immediates (16-bit) wherever the same width applies. */
#define MIN_8BIT_SIGNED (-128)
#define MAX_8BIT_SIGNED 127
#define MIN_16BIT_SIGNED (-32768)
#define MAX_16BIT_SIGNED 32767
#define MIN_32BIT_SIGNED (-2147483647L - 1)
#define MAX_32BIT_SIGNED 2147483647L

/* Error messages: label/symbol-name validation (extractLabel,
 * parseSymbolOperand, and eventually operand label references all share
 * this via validateSymbolToken). */
#define ERR_SYMBOL_TOO_LONG "Symbol name exceeds maximum length"
#define ERR_SYMBOL_INVALID_SYNTAX "Invalid symbol name syntax"
#define ERR_SYMBOL_RESERVED_WORD "Symbol name is a reserved word"
#define ERR_SYMBOL_MISSING "Missing symbol name"
#define ERR_SYMBOL_TRAILING_CHARS "Extra characters after symbol name"
#define ERR_SYMBOL_IS_MACRO_NAME "Symbol name is also a macro name"
#define ERR_DUPLICATE_LABEL "Duplicate label"
#define ERR_SYMBOL_ATTRIBUTE_CONFLICT "Symbol already defined with a different attribute"
#define ERR_LABEL_WITHOUT_STATEMENT "Label must be followed by an instruction or directive"

/* Error messages: data directives (.db/.dw/.dh/.asciz) */
#define ERR_VALUE_LIST_MISSING "Missing value list"
#define ERR_EXPECTED_NUMBER "Expected a number"
#define ERR_VALUE_OUT_OF_RANGE_FMT "Value out of range for %s"
#define ERR_EXPECTED_COMMA_VALUES "Expected ',' between values"
#define ERR_EXPECTED_VALUE_AFTER_COMMA "Expected a value after ','"
#define ERR_ASCIZ_MISSING_OPEN_QUOTE "Expected '\"' to start .asciz string"
#define ERR_ASCIZ_NON_PRINTABLE "String contains a non-printable character"
#define ERR_ASCIZ_MISSING_CLOSE_QUOTE "Missing closing '\"' for .asciz string"
#define ERR_ASCIZ_TRAILING_CHARS "Extra characters after .asciz string"

/* Error messages: operation/directive line dispatch */
#define ERR_UNKNOWN_DIRECTIVE "Unknown directive"
#define ERR_UNKNOWN_OPERATION "Unknown operation"

/* Error messages: operation operands */
#define ERR_EXPECTED_REGISTER "Expected a register operand"
#define ERR_REGISTER_OUT_OF_RANGE "Register number out of range (0-31)"
#define ERR_IMMEDIATE_OUT_OF_RANGE "Immediate value out of range (-32768 to 32767)"
#define ERR_EXPECTED_COMMA_OPERANDS "Expected ',' between operands"
#define ERR_TRAILING_OPERAND_CHARS "Extra characters after operands"

/* Error messages: general */
#define ERR_OUT_OF_MEMORY "Out of memory"

/* Complete Operation enum with all instructions from the ISA */
typedef enum {
    /* R-type instructions */
    OP_ADD,        /* opcode: 0, funct: 1 */
    OP_SUB,        /* opcode: 0, funct: 2 */
    OP_AND,        /* opcode: 0, funct: 3 */
    OP_OR,         /* opcode: 0, funct: 4 */
    OP_NOR,        /* opcode: 0, funct: 5 */
    OP_MOVE,       /* opcode: 1, funct: 1 */
    OP_MVHI,       /* opcode: 1, funct: 2 */
    OP_MVLO,       /* opcode: 1, funct: 3 */
    
    /* I-type instructions */
    OP_ADDI,       /* opcode: 10 */
    OP_SUBI,       /* opcode: 11 */
    OP_ANDI,       /* opcode: 12 */
    OP_ORI,        /* opcode: 13 */
    OP_NORI,       /* opcode: 14 */
    OP_BNE,        /* opcode: 15 */
    OP_BEQ,        /* opcode: 16 */
    OP_BLT,        /* opcode: 17 */
    OP_BGT,        /* opcode: 18 */
    OP_LB,         /* opcode: 19 */
    OP_SB,         /* opcode: 20 */
    OP_LW,         /* opcode: 21 */
    OP_SW,         /* opcode: 22 */
    OP_LH,         /* opcode: 23 */
    OP_SH,         /* opcode: 24 */
    
    /* J-type instructions */
    OP_JMP,        /* opcode: 30 */
    OP_LA,         /* opcode: 31 */
    OP_CALL,       /* opcode: 32 */
    OP_HLT,        /* opcode: 63 */
    
    OP_INVALID = -1
} Operation;

typedef enum {
    DIRECTIVE_ENTRY,
    DIRECTIVE_EXTERN,
    DIRECTIVE_DB,
    DIRECTIVE_DW,
    DIRECTIVE_DH,
    DIRECTIVE_ASCIZ,
    DIRECTIVE_INVALID = -1
} Directive;

/**
 * Converts an opcode name to its Operation enum value.
 * @param str the string to convert (e.g., "add", "addi", "jmp")
 * @return the Operation value, or OP_INVALID if not recognized
 */
Operation operationFromString(const char *str);

/**
 * Converts an Operation enum value to its opcode name.
 * @param op the operation enum value
 * @return the operation name, or NULL if invalid
 */
const char* operationToString(Operation op);

/**
 * Converts a directive name to its Directive enum value.
 * @param str the string to convert, including the leading dot (e.g., ".entry")
 * @return the Directive value, or DIRECTIVE_INVALID if not recognized
 */
Directive directiveFromString(const char *str);

/**
 * Converts a Directive enum value to its directive name.
 * @param directive the directive enum value
 * @return the directive name with its leading dot, or NULL if invalid
 */
const char* directiveToString(Directive directive);

/**
 * Looks up the opcode field value for an operation.
 * @param op the operation enum value
 * @return the opcode (0-63), or -1 if invalid
 */
int getOpcode(Operation op);

/**
 * Looks up the funct field value for an operation.
 * @param op the operation enum value
 * @return the funct value for R-type operations, 0 for I/J-type, -1 if invalid
 */
int getFunct(Operation op);

/**
 * Looks up which instruction format an operation uses.
 * @param op the operation enum value
 * @return 'R', 'I', or 'J', or '\0' if invalid
 */
char getOperationType(Operation op);

#endif /* COMMON_H */
