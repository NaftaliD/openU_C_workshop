#ifndef COMMON_H
#define COMMON_H

/* Command enumeration - all supported commands */
typedef enum {
    CMD_READ_COMP,
    CMD_PRINT_COMP,
    CMD_ADD_COMP,
    CMD_SUB_COMP,
    CMD_MULT_COMP_REAL,
    CMD_MULT_COMP_IMG,
    CMD_MULT_COMP_COMP,
    CMD_ABS_COMP,
    CMD_STOP,
    CMD_INVALID
} CommandType;

/* Input read result codes */
typedef enum {
    INPUT_EOF = 0,      /* EOF encountered */
    INPUT_VALID = 1,    /* Valid line read */
    INPUT_INVALID = 2   /* Invalid line, retry */
} InputReadResult;

/* Command execution result codes */
typedef enum {
    COMMAND_EXIT = 0,    /* Exit program */
    COMMAND_CONTINUE = 1 /* Continue processing */
} CommandResult;

/* Error message constants */
#define ERR_UNDEFINED_COMMAND "Undefined command name"
#define ERR_MISSING_PARAMETER "Missing parameter"
#define ERR_EXTRANEOUS_TEXT "Extraneous text after end of command"
#define ERR_INVALID_NUMBER "Invalid parameter - not a number"
#define ERR_MULTIPLE_COMMAS "Multiple consecutive commas"
#define ERR_ILLEGAL_COMMA "Illegal comma"
#define ERR_UNDEFINED_VARIABLE "Undefined complex variable"
#define ERR_EOF_NO_STOP "EOF encountered - program termination required"
#define ERR_LINE_TOO_LONG "Input line exceeds maximum length of 80 characters"

/* System constants */
#define MAX_LINE_LENGTH 80
#define MAX_INPUT_BUFFER_SIZE 256
#define NUM_COMPLEX_VARS 6
#define MIN_DECIMAL_PLACES 2
#define MAX_COMMAND_NAME_LENGTH 32
#define MAX_ARG_LENGTH 80
#define MAX_ARGS 3

/* Variable name constants */
#define VAR_A 'A'
#define VAR_B 'B'
#define VAR_C 'C'
#define VAR_D 'D'
#define VAR_E 'E'
#define VAR_F 'F'

/* Delimiter constants */
#define COMMA_DELIMITER ','
#define COMMA_STRING ","
#define DOUBLE_COMMA ",,"

/**
 * Removes leading and trailing whitespace from the given string.
 * @param str string to trim in place
 */
void trim_whitespace(char *str);

/**
 * Determines whether the string contains only whitespace characters.
 * @param str string to test
 * @return 1 if the string is blank or NULL, 0 otherwise
 */
int is_blank_string(const char *str);

/* Command name strings for parsing - declared extern, defined in mycomp.c */
extern const char* COMMAND_STRINGS[];
extern const int NUM_COMMANDS;

/* User interface strings */
#define PROMPT_ENTER_COMMAND "Enter command: "
#define OUTPUT_FORMAT_COMPLEX "%.2f + (%.2f)i"
#define OUTPUT_FORMAT_REAL "%.2f"

#endif /* COMMON_H */