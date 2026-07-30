/*
 * operation_line.h
 * A single parsed operation statement (add, jmp, hlt, ...). The list of them
 * is a thin wrapper around the generic PointerList (see pointer_list.h).
 */

#ifndef OPERATION_LINE_H
#define OPERATION_LINE_H

#include "common.h"
#include "pointer_list.h"

typedef enum {
    OPERAND_REGISTER,
    OPERAND_IMMEDIATE,
    OPERAND_LABEL_REF   /* resolved by Pass Two via the symbol table */
} OperandType;

typedef union {
    int  register_number;
    long immediate_value;  /* long, not int: C90 guarantees only 16 bits for
                               int, but .dw needs full 32-bit signed values */
    char label_name[MAX_LABEL_LENGTH + 1];
} OperandValue;

typedef struct {
    OperandType  type;
    OperandValue value;
} Operand;

/* One fully-parsed operation statement */
typedef struct {
    int       line_number;
    Operation op;
    int       operand_count;
    Operand   operands[MAX_OPERANDS]; /* source syntax order, not bit-field order */
    int       address;                /* IC when encountered; frozen once stamped */
    int       has_error;
} OperationNode;

typedef PointerList OperationList;

/**
 * Allocates and initializes an empty operation list.
 * @return the new list, or NULL on allocation failure
 */
OperationList* createOperationList(void);

/**
 * Appends node, growing the list if needed. Takes ownership of node.
 * @param list the operation list
 * @param node the node to append
 * @return 0 on success, -1 on allocation failure
 */
int appendOperation(OperationList *list, OperationNode *node);

/**
 * Frees every node, the backing array, and the list itself.
 * @param list the operation list to free
 */
void destroyOperationList(OperationList *list);

#endif /* OPERATION_LINE_H */
