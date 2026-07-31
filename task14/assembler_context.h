/*
 * assembler_context.h
 * Cross-step state for assembling one .as file -- created once, threaded
 * through macro expansion / Pass One / (later) Pass Two, destroyed once
 * after the whole pipeline for that file finishes.
 */

#ifndef ASSEMBLER_CONTEXT_H
#define ASSEMBLER_CONTEXT_H

#include "macro_table.h"
#include "symbol_table.h"
#include "operation_line.h"
#include "data_directive_line.h"
#include "entry_list.h"
#include "error_list.h"

typedef struct {
    MacroTable        *macros;      /* set by expandMacros on success; NULL until then */
    SymbolTable       *symbols;
    OperationList     *operations;
    DataDirectiveList *directives;
    EntryList         *entries;
    ErrorList         *errors;
    int                icf;
    int                dcf;
} AssemblerContext;

/**
 * Allocates a context with every list/table created empty (macros starts
 * NULL; expandMacros fills it in).
 * @return the new context, or NULL on allocation failure
 */
AssemblerContext* createAssemblerContext(void);

/**
 * Frees everything held by the context, then the context itself.
 * @param context the context to free
 */
void destroyAssemblerContext(AssemblerContext *context);

#endif /* ASSEMBLER_CONTEXT_H */
