/*
 * assembler_context.c
 * Implementation of AssemblerContext.
 */

#include <stdlib.h>
#include "assembler_context.h"

AssemblerContext* createAssemblerContext(void)
{
    AssemblerContext *context = (AssemblerContext *)malloc(sizeof(AssemblerContext));
    if (context == NULL) {
        return NULL;
    }

    context->macros = NULL;
    context->symbols = createSymbolTable();
    context->operations = createOperationList();
    context->directives = createDataDirectiveList();
    context->entries = createEntryList();
    context->errors = createErrorList();
    context->icf = 0;
    context->dcf = 0;

    if (context->symbols == NULL || context->operations == NULL ||
        context->directives == NULL || context->entries == NULL ||
        context->errors == NULL) {
        destroyAssemblerContext(context);
        return NULL;
    }

    return context;
}

void destroyAssemblerContext(AssemblerContext *context)
{
    if (context == NULL) {
        return;
    }

    destroyMacroTable(context->macros);
    destroySymbolTable(context->symbols);
    destroyOperationList(context->operations);
    destroyDataDirectiveList(context->directives);
    destroyEntryList(context->entries);
    destroyErrorList(context->errors);
    free(context);
}
