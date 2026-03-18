#include <stdio.h>
#include "common.h"
#include "utils.h"

int main() {
    printf("Testing Operation Encoding Functions\n");
    printf("=====================================\n\n");

    /* Test R-type instructions */
    printf("R-Type Instructions:\n");
    printf("OP_ADD: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_ADD), getFunct(OP_ADD), getOperationType(OP_ADD));
    printf("OP_SUB: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_SUB), getFunct(OP_SUB), getOperationType(OP_SUB));
    printf("OP_AND: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_AND), getFunct(OP_AND), getOperationType(OP_AND));
    printf("OP_NOR: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_NOR), getFunct(OP_NOR), getOperationType(OP_NOR));
    printf("OP_MOVE: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_MOVE), getFunct(OP_MOVE), getOperationType(OP_MOVE));
    printf("OP_MVHI: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_MVHI), getFunct(OP_MVHI), getOperationType(OP_MVHI));
    printf("\n");

    /* Test I-type instructions */
    printf("I-Type Instructions:\n");
    printf("OP_ADDI: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_ADDI), getFunct(OP_ADDI), getOperationType(OP_ADDI));
    printf("OP_BEQ: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_BEQ), getFunct(OP_BEQ), getOperationType(OP_BEQ));
    printf("OP_LW: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_LW), getFunct(OP_LW), getOperationType(OP_LW));
    printf("OP_SW: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_SW), getFunct(OP_SW), getOperationType(OP_SW));
    printf("OP_SH: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_SH), getFunct(OP_SH), getOperationType(OP_SH));
    printf("\n");

    /* Test J-type instructions */
    printf("J-Type Instructions:\n");
    printf("OP_JMP: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_JMP), getFunct(OP_JMP), getOperationType(OP_JMP));
    printf("OP_LA: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_LA), getFunct(OP_LA), getOperationType(OP_LA));
    printf("OP_CALL: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_CALL), getFunct(OP_CALL), getOperationType(OP_CALL));
    printf("OP_HLT: opcode=%d, funct=%d, type=%c\n", getOpcode(OP_HLT), getFunct(OP_HLT), getOperationType(OP_HLT));

    return 0;
}
