/*
 * common.c
 * Common definitions and ISA-related function implementations.
 */

#include <string.h>
#include "common.h"

/* Operation strings for the complete ISA */
static const char *OPERATION_STRINGS[] = {
    /* R-type instructions */
    "add", "sub", "and", "or", "nor", "move", "mvhi", "mvlo",
    /* I-type instructions */
    "addi", "subi", "andi", "ori", "nori", "bne", "beq", "blt", "bgt",
    "lb", "sb", "lw", "sw", "lh", "sh",
    /* J-type instructions */
    "jmp", "la", "call", "hlt"
};

/* Derived from the array itself so it can never drift out of sync again */
static const int OPERATION_COUNT = sizeof(OPERATION_STRINGS) / sizeof(OPERATION_STRINGS[0]);

/* Directive strings */
static const char *DIRECTIVE_STRINGS[] = {
    ".entry", ".extern", ".db", ".dw", ".dh", ".asciz"
};

static const int DIRECTIVE_COUNT = sizeof(DIRECTIVE_STRINGS) / sizeof(DIRECTIVE_STRINGS[0]);

Operation OperationFromString(const char *str)
{
    int i;
    
    if (str == NULL || strlen(str) == 0) {
        return OP_INVALID;
    }
    
    for (i = 0; i < OPERATION_COUNT; i++) {
        if (strcmp(str, OPERATION_STRINGS[i]) == 0) {
            return (Operation)i;
        }
    }
    
    return OP_INVALID;
}

const char* OperationToString(Operation op)
{
    if (op < 0 || op >= OPERATION_COUNT) {
        return NULL;
    }
    
    return OPERATION_STRINGS[op];
}

Directive DirectiveFromString(const char *str)
{
    int i;
    
    if (str == NULL || strlen(str) == 0) {
        return DIRECTIVE_INVALID;
    }
    
    for (i = 0; i < DIRECTIVE_COUNT; i++) {
        if (strcmp(str, DIRECTIVE_STRINGS[i]) == 0) {
            return (Directive)i;
        }
    }
    
    return DIRECTIVE_INVALID;
}

const char* DirectiveToString(Directive directive)
{
    if (directive < 0 || directive >= DIRECTIVE_COUNT) {
        return NULL;
    }
    
    return DIRECTIVE_STRINGS[directive];
}

int getOpcode(Operation op)
{
    switch (op) {
        /* R-type instructions: opcode 0 */
        case OP_ADD:
        case OP_SUB:
        case OP_AND:
        case OP_OR:
        case OP_NOR:
            return 0;
        
        /* R-type instructions: opcode 1 */
        case OP_MOVE:
        case OP_MVHI:
        case OP_MVLO:
            return 1;
        
        /* I-type instructions */
        case OP_ADDI:
            return 10;
        case OP_SUBI:
            return 11;
        case OP_ANDI:
            return 12;
        case OP_ORI:
            return 13;
        case OP_NORI:
            return 14;
        case OP_BNE:
            return 15;
        case OP_BEQ:
            return 16;
        case OP_BLT:
            return 17;
        case OP_BGT:
            return 18;
        case OP_LB:
            return 19;
        case OP_SB:
            return 20;
        case OP_LW:
            return 21;
        case OP_SW:
            return 22;
        case OP_LH:
            return 23;
        case OP_SH:
            return 24;
        
        /* J-type instructions */
        case OP_JMP:
            return 30;
        case OP_LA:
            return 31;
        case OP_CALL:
            return 32;
        case OP_HLT:
            return 63;
        
        default:
            return -1;
    }
}

int getFunct(Operation op)
{
    switch (op) {
        /* R-type: opcode 0 functions */
        case OP_ADD:
            return 1;
        case OP_SUB:
            return 2;
        case OP_AND:
            return 3;
        case OP_OR:
            return 4;
        case OP_NOR:
            return 5;
        
        /* R-type: opcode 1 functions */
        case OP_MOVE:
            return 1;
        case OP_MVHI:
            return 2;
        case OP_MVLO:
            return 3;
        
        /* I-type and J-type have no funct field */
        case OP_ADDI:
        case OP_SUBI:
        case OP_ANDI:
        case OP_ORI:
        case OP_NORI:
        case OP_BNE:
        case OP_BEQ:
        case OP_BLT:
        case OP_BGT:
        case OP_LB:
        case OP_SB:
        case OP_LW:
        case OP_SW:
        case OP_LH:
        case OP_SH:
        case OP_JMP:
        case OP_LA:
        case OP_CALL:
        case OP_HLT:
            return 0;
        
        default:
            return -1;
    }
}

char getOperationType(Operation op)
{
    switch (op) {
        /* R-type instructions */
        case OP_ADD:
        case OP_SUB:
        case OP_AND:
        case OP_OR:
        case OP_NOR:
        case OP_MOVE:
        case OP_MVHI:
        case OP_MVLO:
            return 'R';
        
        /* I-type instructions */
        case OP_ADDI:
        case OP_SUBI:
        case OP_ANDI:
        case OP_ORI:
        case OP_NORI:
        case OP_BNE:
        case OP_BEQ:
        case OP_BLT:
        case OP_BGT:
        case OP_LB:
        case OP_SB:
        case OP_LW:
        case OP_SW:
        case OP_LH:
        case OP_SH:
            return 'I';
        
        /* J-type instructions */
        case OP_JMP:
        case OP_LA:
        case OP_CALL:
        case OP_HLT:
            return 'J';
        
        default:
            return '\0';
    }
}
