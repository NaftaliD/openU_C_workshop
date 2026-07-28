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

/* Operation parsing and encoding functions */

/**
 * OperationFromString - Convert a string to an Operation enum
 * @str: The string to convert (e.g., "add", "addi", "jmp")
 * Returns: The Operation enum value, or OP_INVALID if not recognized
 */
Operation OperationFromString(const char *str);

/**
 * OperationToString - Convert an Operation enum to its string representation
 * @op: The operation enum value
 * Returns: The operation name as a string, or NULL if invalid
 */
const char* OperationToString(Operation op);

/**
 * DirectiveFromString - Convert a string to a Directive enum
 * @str: The string to convert (must include leading dot, e.g., ".entry")
 * Returns: The Directive enum value, or DIRECTIVE_INVALID if not recognized
 */
Directive DirectiveFromString(const char *str);

/**
 * DirectiveToString - Convert a Directive enum to its string representation
 * @directive: The directive enum value
 * Returns: The directive name as a string (with leading dot), or NULL if invalid
 */
const char* DirectiveToString(Directive directive);

/**
 * getOpcode - Get the opcode value for an Operation
 * @op: The operation enum member
 * Returns: The opcode value (0-63), or -1 if invalid
 */
int getOpcode(Operation op);

/**
 * getFunct - Get the funct value for an Operation (R-type only)
 * @op: The operation enum member
 * Returns: The funct value (1-5 for R-type), 0 for I/J-type, or -1 if invalid
 */
int getFunct(Operation op);

/**
 * getOperationType - Get the operation type for an Operation
 * @op: The operation enum member
 * Returns: 'R', 'I', or 'J', or '\0' if invalid
 */
char getOperationType(Operation op);

#endif /* COMMON_H */
