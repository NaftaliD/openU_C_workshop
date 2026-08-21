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
#define MAX_FILENAME_LENGTH 256  /* Ubuntu/Linux NAME_MAX is 255 bytes; +1 for the null terminator */

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
#define MAX_MEMORY_SIZE 33554432 /* 2^25 addressable bytes, i.e. valid addresses 0..2^25-1 */

#define MIN_REGISTER_NUMBER 0
#define MAX_REGISTER_NUMBER 31

#define INSTRUCTION_SIZE_BYTES 4     /* every instruction is 32 bits, always */
#define INITIAL_MACRO_BODY_CAPACITY 256
#define AS_FILE_EXTENSION ".as"
#define AM_FILE_EXTENSION ".am"
#define OB_FILE_EXTENSION ".ob"
#define ENT_FILE_EXTENSION ".ent"
#define EXT_FILE_EXTENSION ".ext"

/* Debug-dump (*ToString) sizing, shared by every module's debug-string
 * builder (symbol_table.c, entry_list.c, operation_line.c,
 * data_directive_line.c, macro_table.c, error_list.c). */
#define BUFF_SIZE 1024              /* stack buffer for a single node/entry dump */
#define LARGE_BUFF_SIZE (BUFF_SIZE * 8) /* stack buffer for a list-level dump (many entries concatenated) */
#define NULL_PLACEHOLDER_SIZE 32    /* malloc size for a "[Xxx: NULL]\n" string */
#define MAX_INT_DIGITS 11           /* worst case %d for a 32-bit int, incl. sign */
#define MAX_LONG_DIGITS 21          /* worst case %ld for a 32-bit signed long, incl. sign */
#define MAX_OPERAND_STRING_LENGTH (MAX_LABEL_LENGTH + 1) /* longest of $N / immediate / label text */
#define ENTRY_DELIMITER_OVERHEAD 32 /* headroom for literal punctuation/brackets per appended entry */
#define ERROR_LIST_MARGIN (MAX_ERROR_MESSAGE_LENGTH + MAX_INT_DIGITS + ENTRY_DELIMITER_OVERHEAD)
#define SIMPLE_LINE_MARGIN (MAX_LABEL_LENGTH + MAX_INT_DIGITS + ENTRY_DELIMITER_OVERHEAD)
#define VALUE_LIST_MARGIN (MAX_LONG_DIGITS + ENTRY_DELIMITER_OVERHEAD)
#define NODE_DUMP_MARGIN (MAX_OPERANDS * MAX_OPERAND_STRING_LENGTH + MAX_INT_DIGITS * 2 + ENTRY_DELIMITER_OVERHEAD)

/* Replaces each function's own ad hoc int return convention (see README.md). */
typedef enum {
    FALSE = 0,
    TRUE = 1
} Boolean;

typedef enum {
    RC_SUCCESS = 0,
    RC_DUPLICATE,
    RC_OUT_OF_MEMORY,
    RC_NOT_FOUND,
    RC_CONFLICT,
    RC_INVALID,
    RC_IO_ERROR
} ResultCode;

/* Bakes a macro's numeric value into a string literal at compile time, e.g.
 * for embedding a range limit into an error message with no runtime cost. */
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

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
#define ERR_MEMORY_OVERFLOW "Program exceeds maximum memory size (" STRINGIFY(MAX_MEMORY_SIZE) " bytes)"

/* Error messages: data directives (.db/.dw/.dh/.asciz) */
#define ERR_VALUE_LIST_MISSING "Missing value list"
#define ERR_EXPECTED_NUMBER "Expected a number"
#define ERR_VALUE_OUT_OF_RANGE "Value out of range for .%s"
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
#define ERR_REGISTER_OUT_OF_RANGE \
    "Register number out of range (" STRINGIFY(MIN_REGISTER_NUMBER) "-" STRINGIFY(MAX_REGISTER_NUMBER) ")"
#define ERR_IMMEDIATE_OUT_OF_RANGE \
    "Immediate value out of range (" STRINGIFY(MIN_16BIT_SIGNED) " to " STRINGIFY(MAX_16BIT_SIGNED) ")"
#define ERR_EXPECTED_COMMA_OPERANDS "Expected ',' between operands"
#define ERR_TRAILING_OPERAND_CHARS "Extra characters after operands"

/* Error messages: Pass Two (pass_two.c) */
#define ERR_UNDEFINED_SYMBOL "Undefined symbol"
#define ERR_BRANCH_EXTERNAL "Branch target cannot be an external symbol"
#define ERR_BRANCH_OUT_OF_RANGE \
    "Branch distance out of range (" STRINGIFY(MIN_16BIT_SIGNED) " to " STRINGIFY(MAX_16BIT_SIGNED) ")"
#define ERR_CANNOT_WRITE_EXT_FILE "Could not write to the .ext file"
#define ERR_CANNOT_WRITE_ENT_FILE "Could not write to the .ent file"

/* Error messages: general */
#define ERR_OUT_OF_MEMORY "Out of memory"

/* Error messages: macro pre-assembler (macro_processor.c). Full logMessage
 * format strings, not plain ErrorList messages like the ones above. */
#define ERR_MACRO_CANNOT_OPEN_INPUT "Cannot open input file %s\n"
#define ERR_MACRO_CANNOT_OPEN_OUTPUT "Cannot open output file %s\n"
#define ERR_MACRO_TABLE_ALLOC_FAILED "Cannot allocate macro table\n"
#define ERR_MACRO_BODY_RESIZE_FAILED "Cannot resize macro body buffer\n"
#define ERR_MCROEND_TRAILING_CHARS "At line %d: Extra characters after 'mcroend'\n"
#define ERR_MCROEND_WITHOUT_MCRO "At line %d: 'mcroend' without matching 'mcro'\n"
#define ERR_MACRO_ALREADY_DEFINED "At line %d: Macro '%s' already defined\n"
#define ERR_MACRO_REGISTER_ALLOC_FAILED "Cannot allocate memory for macro '%s'\n"
#define ERR_MACRO_REGISTER_UNEXPECTED "Unexpected internal error registering macro '%s'\n"
#define ERR_MACRO_NESTED_DEFINITION "At line %d: Nested macro definition ('mcro' inside macro '%s', missing 'mcroend')\n"
#define ERR_MCRO_INVALID_SYNTAX "At line %d: Invalid macro definition syntax. Expected: mcro <name>\n"
#define ERR_MACRO_NAME_INVALID "At line %d: Macro name '%s' is invalid (must start with a letter and contain only letters/digits/underscores)\n"
#define ERR_MACRO_NAME_RESERVED "At line %d: Macro name '%s' is a reserved word\n"
#define ERR_MACRO_BODY_ALLOC_FAILED "Cannot allocate macro body buffer\n"
#define ERR_MACRO_LINE_ALLOC_FAILED "Cannot allocate memory while processing line %d\n"
#define ERR_LINE_TOO_LONG "At line %d: Line exceeds maximum length of %d characters\n"
#define ERR_MACRO_UNCLOSED "Unclosed macro '%s' - missing 'mcroend' at end of file\n"

/* Error messages: pipeline orchestration (assembler_core.c), same
 * full-string convention as above. */
#define ERR_MACRO_PROCESSOR_FAILED "Macro processor failed for %s\n"
#define ERR_MACRO_PROCESSOR_IO_FAILED "Could not process %s (file I/O error)\n"
#define ERR_PASS_ONE_OPEN_FAILED "Could not open %s for Pass One\n"
#define ERR_PASS_ONE_FAILED "Pass One failed for %s (%d error(s))\n"
#define ERR_PASS_TWO_OPEN_FAILED "Could not open %s for Pass Two\n"
#define ERR_PASS_TWO_FAILED "Pass Two failed for %s (%d error(s))\n"
#define ERR_MISSING_AS_EXTENSION "%s does not have a .as extension\n"
#define ERR_FILE_OUT_OF_MEMORY "Out of memory processing %s\n"
#define ERR_FILENAME_TOO_LONG "Filename too long: %s\n"
#define USAGE_MESSAGE "Usage: %s <file1.as> [file2.as] ...\n"

/* Diagnostic messages: logger.c */
#define WARN_LOG_FILE_OPEN_FAILED "Warning: Cannot open debug log file %s\n"

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
 * @param str the string to convert, without the leading dot (e.g., "entry")
 * @return the Directive value, or DIRECTIVE_INVALID if not recognized
 */
Directive directiveFromString(const char *str);

/**
 * Converts a Directive enum value to its directive name.
 * @param directive the directive enum value
 * @return the directive name without its leading dot, or NULL if invalid
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
