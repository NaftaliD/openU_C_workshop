/*
 * operation_line.h
 * A single parsed operation statement (add, jmp, hlt, ...), and a growable
 * list of them (backed by growable_array.h's shared growth primitive).
 */

#ifndef OPERATION_LINE_H
#define OPERATION_LINE_H

#include "common.h"

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

/* Growable array of pointers, in file order */
typedef struct {
    OperationNode **items;
    int count;
    int capacity;
} OperationList;

/**
 * Allocates an OperationNode. Fields are left uninitialized -- the caller
 * (parseLine's operation sub-parser) fills in every field unconditionally.
 * @return the new node, or NULL on allocation failure
 */
OperationNode* createOperationNode(void);

/**
 * Allocates and initializes an empty operation list.
 * @return the new list, or NULL on allocation failure
 */
OperationList* createOperationList(void);

/**
 * Appends node, growing the list if needed. Takes ownership of node.
 * @param list the operation list
 * @param node the node to append
 * @return TRUE on success, FALSE on allocation failure
 */
Boolean appendOperation(OperationList *list, OperationNode *node);

/**
 * Frees every node, the backing array, and the list itself.
 * @param list the operation list to free
 */
void destroyOperationList(OperationList *list);

/**
 * Builds a debug string describing one operation node. Caller must free the
 * returned string.
 * @param node the node to describe
 * @return a malloc'd description string
 */
char* operationNodeToString(const OperationNode *node);

/**
 * Builds a debug string describing every node in the list. Caller must free
 * the returned string.
 * @param list the operation list to describe
 * @return a malloc'd description string
 */
char* operationListToString(const OperationList *list);

#endif /* OPERATION_LINE_H */
