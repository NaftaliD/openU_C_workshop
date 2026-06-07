#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "complex.h"

/* Command name strings for parsing */
const char* COMMAND_STRINGS[] = {
    "read_comp",      /* CMD_READ_COMP */
    "print_comp",     /* CMD_PRINT_COMP */
    "add_comp",       /* CMD_ADD_COMP */
    "sub_comp",       /* CMD_SUB_COMP */
    "mult_comp_real", /* CMD_MULT_COMP_REAL */
    "mult_comp_img",  /* CMD_MULT_COMP_IMG */
    "mult_comp_comp", /* CMD_MULT_COMP_COMP */
    "abs_comp",       /* CMD_ABS_COMP */
    "stop"            /* CMD_STOP */
};

const int NUM_COMMANDS = sizeof(COMMAND_STRINGS) / sizeof(COMMAND_STRINGS[0]);

/**
 * Retrieves a pointer to a complex variable by its name.
 * @param variables array of complex variables
 * @param var_name variable name (A-F)
 * @return pointer to the matching complex variable or NULL if undefined
 */
complex* get_variable_by_name(complex variables[], char var_name) {
    switch (var_name) {
        case VAR_A: return &variables[0];
        case VAR_B: return &variables[1];
        case VAR_C: return &variables[2];
        case VAR_D: return &variables[3];
        case VAR_E: return &variables[4];
        case VAR_F: return &variables[5];
        default: return NULL;
    }
}

/**
 * Maps a command string to its CommandType enumeration value.
 * @param command_str the command text to translate
 * @return corresponding CommandType or CMD_INVALID if unknown
 */
CommandType parse_command_type(const char* command_str) {
    int i;
    for (i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(command_str, COMMAND_STRINGS[i]) == 0) {
            return (CommandType)i;
        }
    }
    return CMD_INVALID;
}

typedef struct {
    CommandType type;
    int argc;
    char argv[MAX_ARGS][MAX_ARG_LENGTH + 1];
} CommandLine;

/**
 * Initializes all complex variables to zero.
 * @param variables array of complex variables to initialize
 */
void initialize_variables(complex variables[]) {
    int i;
    for (i = 0; i < NUM_COMPLEX_VARS; i++) {
        init_complex_zero(&variables[i]);
    }
}

/**
 * Parses the command name at the start of a buffer.
 * @param buffer input buffer to scan; modified in place
 * @param cmdline output command metadata
 * @param rest receives pointer to the first argument after the command
 * @return 0 on success, 1 on failure
 */
static int parse_command_name(char *buffer, CommandLine *cmdline, char **rest) {
    char *cursor = buffer;

    while (*cursor != '\0' && !isspace((unsigned char)*cursor)) {
        cursor++;
    }

    if (cursor == buffer || cursor - buffer >= MAX_COMMAND_NAME_LENGTH) {
        return 1;
    }

    if (*cursor != '\0') {
        *cursor = '\0';
        cursor++;
        while (*cursor != '\0' && isspace((unsigned char)*cursor)) {
            cursor++;
        }
    }

    cmdline->type = parse_command_type(buffer);
    if (cmdline->type == CMD_INVALID) {
        return 1;
    }

    *rest = cursor;
    return 0;
}

/**
 * Parses a numeric argument into a double value.
 * @param token input token, trimmed of whitespace
 * @param out parsed value destination
 * @return 1 if parsing succeeded, 0 otherwise
 */
static int parse_double(const char *token, double *out) {
    char *endptr;
    double value;

    if (token == NULL || *token == '\0') {
        return 0;
    }

    value = strtod(token, &endptr);
    if (endptr == token) {
        return 0;
    }

    while (*endptr != '\0') {
        if (!isspace((unsigned char)*endptr)) {
            return 0;
        }
        endptr++;
    }

    *out = value;
    return 1;
}

/**
 * Resolves a variable token to a complex variable pointer.
 * @param token input token, trimmed of whitespace
 * @param variables array of complex variables
 * @return pointer to matching variable or NULL if invalid
 */
static complex* parse_complex_variable(const char *token, complex variables[]) {
    if (token == NULL || strlen(token) != 1) {
        return NULL;
    }
    return get_variable_by_name(variables, token[0]);
}

/**
 * Validates and retrieves a complex variable, printing appropriate error.
 * If token is too long, reports extraneous text; if not found, reports undefined.
 * @param token the token to parse as variable name
 * @param variables array of complex variables
 * @return pointer to variable or NULL on error (error message already printed)
 */
static complex* validate_and_get_variable(const char *token, complex variables[]) {
    complex *result;

    if (token == NULL) {
        printf("%s\n", ERR_UNDEFINED_VARIABLE);
        return NULL;
    }

    if (strlen(token) > 1) {
        printf("%s\n", ERR_EXTRANEOUS_TEXT);
        return NULL;
    }

    result = parse_complex_variable(token, variables);
    if (result == NULL) {
        printf("%s\n", ERR_UNDEFINED_VARIABLE);
    }
    return result;
}

/**
 * Returns the expected number of arguments for a given command.
 * @param cmd command type
 * @return expected argc for the command
 */
static int expected_arg_count(CommandType cmd) {
    switch (cmd) {
        case CMD_READ_COMP: return 3;
        case CMD_PRINT_COMP: return 1;
        case CMD_ADD_COMP: return 2;
        case CMD_SUB_COMP: return 2;
        case CMD_MULT_COMP_REAL: return 2;
        case CMD_MULT_COMP_IMG: return 2;
        case CMD_MULT_COMP_COMP: return 2;
        case CMD_ABS_COMP: return 1;
        case CMD_STOP: return 0;
        default: return -1;
    }
}

/**
 * Validates argc against the expected count for a command and prints
 * the appropriate error message on mismatch.
 * @param cmd command type
 * @param argc provided argument count
 * @return 1 if valid, 0 if invalid (and an error was printed)
 */
static int validate_arg_count(CommandType cmd, int argc) {
    int expected = expected_arg_count(cmd);
    /* Note: We assume cmd is always valid here because parse_command_line()
     * already validated it. If cmd somehow becomes invalid (unreachable in normal flow),
     * expected_arg_count() returns -1 and we treat it as invalid. */
    if (expected < 0) {
        printf("%s\n", ERR_UNDEFINED_COMMAND);
        return 0;
    }
    if (argc < expected) {
        printf("%s\n", ERR_MISSING_PARAMETER);
        return 0;
    }
    if (argc > expected) {
        printf("%s\n", ERR_EXTRANEOUS_TEXT);
        return 0;
    }
    return 1;
}

/**
 * Parses a full command line into a command type and argument list.
 * @param line input line to parse
 * @param cmdline output command and argument data
 * @return 0 on success, 1 on parse error
 */
static int parse_command_line(const char *line, CommandLine *cmdline) {
    char buffer[MAX_INPUT_BUFFER_SIZE];
    char *rest;
    char *token;
    int arg_index = 0;

    if (line == NULL || cmdline == NULL) {
        return 1;
    }

    strncpy(buffer, line, MAX_INPUT_BUFFER_SIZE - 1);
    buffer[MAX_INPUT_BUFFER_SIZE - 1] = '\0';
    trim_whitespace(buffer);

    if (parse_command_name(buffer, cmdline, &rest) != 0) {
        printf("%s\n", ERR_UNDEFINED_COMMAND);
        return 1;
    }

    /* Trim whitespace from rest */
    trim_whitespace(rest);

    /* Check for comma-related errors before tokenization */
    if (rest[0] == COMMA_DELIMITER) {
        printf("%s\n", ERR_ILLEGAL_COMMA);
        return 1;
    }

    if (strstr(rest, DOUBLE_COMMA) != NULL) {
        printf("%s\n", ERR_MULTIPLE_COMMAS);
        return 1;
    }

    /* Check for trailing comma */
    if (rest[0] != '\0' && rest[strlen(rest) - 1] == COMMA_DELIMITER) {
        printf("%s\n", ERR_MULTIPLE_COMMAS);
        return 1;
    }

    /* Split by comma and parse arguments */
    token = strtok(rest, COMMA_STRING);
    while (token != NULL) {
        trim_whitespace(token);

        if (arg_index >= MAX_ARGS) {
            printf("%s\n", ERR_EXTRANEOUS_TEXT);
            return 1;
        }

        strncpy(cmdline->argv[arg_index], token, MAX_ARG_LENGTH);
        cmdline->argv[arg_index][MAX_ARG_LENGTH] = '\0';
        arg_index++;
        token = strtok(NULL, ",");
    }

    cmdline->argc = arg_index;
    return 0;
}

/**
 * Reads and validates a line of input from stdin.
 * @param buffer destination buffer for the input line
 * @param buffer_size size of the destination buffer
 * @return INPUT_EOF if EOF, INPUT_VALID if valid, INPUT_INVALID if invalid (retry)
 */
InputReadResult read_input_line(char* buffer, size_t buffer_size) {
    size_t len;

    if (fgets(buffer, buffer_size, stdin) == NULL) {
        /* EOF encountered */
        printf("%s\n", ERR_EOF_NO_STOP);
        return INPUT_EOF;
    }

    len = strlen(buffer);
    
    /* Check for line too long: if we got MAX_LINE_LENGTH chars and no newline, line was truncated */
    if (len >= MAX_LINE_LENGTH && buffer[MAX_LINE_LENGTH - 1] != '\n') {
        printf("%s\n", ERR_LINE_TOO_LONG);
        /* Consume rest of line */
        while (getchar() != '\n' && !feof(stdin));
        return INPUT_INVALID;
    }
    
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    if (is_blank_string(buffer)) {
        return INPUT_INVALID; /* Empty lines allowed, just re-prompt */
    }

    return INPUT_VALID;
}

/**
 * Handles the read_comp command.
 * @param variables complex variable storage
 * @param cmdline parsed command and arguments
 */
void process_read_comp(complex variables[], const CommandLine *cmdline) {
    complex *variable;
    double real_part;
    double imag_part;

    /* argc=3 is guaranteed by validate_arg_count() before dispatch */
    variable = parse_complex_variable(cmdline->argv[0], variables);
    if (variable == NULL) {
        printf("%s\n", ERR_UNDEFINED_VARIABLE);
        return;
    }

    if (!parse_double(cmdline->argv[1], &real_part) ||
        !parse_double(cmdline->argv[2], &imag_part)) {
        printf("%s\n", ERR_INVALID_NUMBER);
        return;
    }

    variable->real = real_part;
    variable->imaginary = imag_part;
    print_complex(*variable);
    printf("\n");
}

/**
 * Handles the print_comp command.
 * @param variables complex variable storage
 * @param cmdline parsed command and arguments
 */
void process_print_comp(complex variables[], const CommandLine *cmdline) {
    complex *variable;

    /* argc=1 is guaranteed by validate_arg_count() before dispatch */
    variable = validate_and_get_variable(cmdline->argv[0], variables);
    if (variable == NULL) {
        return;
    }

    print_complex(*variable);
    printf("\n");
}

/**
 * Handles the add_comp command.
 * @param variables complex variable storage
 * @param cmdline parsed command and arguments
 */
void process_add_comp(complex variables[], const CommandLine *cmdline) {
    complex *left;
    complex *right;

    /* argc=2 is guaranteed by validate_arg_count() before dispatch */
    left = validate_and_get_variable(cmdline->argv[0], variables);
    if (left == NULL) {
        return;
    }
    right = validate_and_get_variable(cmdline->argv[1], variables);
    if (right == NULL) {
        return;
    }

    print_complex(add_complex(*left, *right));
    printf("\n");
}

/**
 * Handles the sub_comp command.
 * @param variables complex variable storage
 * @param cmdline parsed command and arguments
 */
void process_sub_comp(complex variables[], const CommandLine *cmdline) {
    complex *left;
    complex *right;

    /* argc=2 is guaranteed by validate_arg_count() before dispatch */
    left = validate_and_get_variable(cmdline->argv[0], variables);
    if (left == NULL) {
        return;
    }
    right = validate_and_get_variable(cmdline->argv[1], variables);
    if (right == NULL) {
        return;
    }

    print_complex(sub_complex(*left, *right));
    printf("\n");
}

/**
 * Handles the mult_comp_real command.
 * @param variables complex variable storage
 * @param cmdline parsed command and arguments
 */
void process_mult_comp_real(complex variables[], const CommandLine *cmdline) {
    complex *variable;
    double multiplier;

    /* argc=2 is guaranteed by validate_arg_count() before dispatch */
    variable = validate_and_get_variable(cmdline->argv[0], variables);
    if (variable == NULL) {
        return;
    }

    if (!parse_double(cmdline->argv[1], &multiplier)) {
        printf("%s\n", ERR_INVALID_NUMBER);
        return;
    }

    print_complex(mult_complex_real(*variable, multiplier));
    printf("\n");
}

/**
 * Handles the mult_comp_img command.
 * @param variables complex variable storage
 * @param cmdline parsed command and arguments
 */
void process_mult_comp_img(complex variables[], const CommandLine *cmdline) {
    complex *variable;
    double coefficient;

    /* argc=2 is guaranteed by validate_arg_count() before dispatch */
    variable = validate_and_get_variable(cmdline->argv[0], variables);
    if (variable == NULL) {
        return;
    }

    if (!parse_double(cmdline->argv[1], &coefficient)) {
        printf("%s\n", ERR_INVALID_NUMBER);
        return;
    }

    print_complex(mult_complex_img(*variable, coefficient));
    printf("\n");
}

/**
 * Handles the mult_comp_comp command.
 * @param variables complex variable storage
 * @param cmdline parsed command and arguments
 */
void process_mult_comp_comp(complex variables[], const CommandLine *cmdline) {
    complex *left;
    complex *right;

    /* argc=2 is guaranteed by validate_arg_count() before dispatch */
    left = validate_and_get_variable(cmdline->argv[0], variables);
    if (left == NULL) {
        return;
    }
    right = validate_and_get_variable(cmdline->argv[1], variables);
    if (right == NULL) {
        return;
    }

    print_complex(mult_complex_complex(*left, *right));
    printf("\n");
}

/**
 * Handles the abs_comp command.
 * @param variables complex variable storage
 * @param cmdline parsed command and arguments
 */
void process_abs_comp(complex variables[], const CommandLine *cmdline) {
    complex *variable;

    /* argc=1 is guaranteed by validate_arg_count() before dispatch */
    variable = validate_and_get_variable(cmdline->argv[0], variables);
    if (variable == NULL) {
        return;
    }

    printf(OUTPUT_FORMAT_REAL "\n", abs_complex(*variable));
}

/**
 * Executes one parsed command line.
 * @param variables complex variable storage
 * @param line raw input command line
 * @return COMMAND_CONTINUE to continue, COMMAND_EXIT to exit
 */
CommandResult process_command(complex variables[], const char* line) {
    CommandLine cmdline;

    if (parse_command_line(line, &cmdline) != 0) {
        return COMMAND_CONTINUE; /* Continue on syntax error */
    }

    /* Validate argc before dispatch */
    if (!validate_arg_count(cmdline.type, cmdline.argc)) {
        return COMMAND_CONTINUE;
    }

    switch (cmdline.type) {
        case CMD_READ_COMP:
            process_read_comp(variables, &cmdline);
            break;
        case CMD_PRINT_COMP:
            process_print_comp(variables, &cmdline);
            break;
        case CMD_ADD_COMP:
            process_add_comp(variables, &cmdline);
            break;
        case CMD_SUB_COMP:
            process_sub_comp(variables, &cmdline);
            break;
        case CMD_MULT_COMP_REAL:
            process_mult_comp_real(variables, &cmdline);
            break;
        case CMD_MULT_COMP_IMG:
            process_mult_comp_img(variables, &cmdline);
            break;
        case CMD_MULT_COMP_COMP:
            process_mult_comp_comp(variables, &cmdline);
            break;
        case CMD_ABS_COMP:
            process_abs_comp(variables, &cmdline);
            break;
        case CMD_STOP:
            return COMMAND_EXIT; /* Exit program (argc already validated) */
        case CMD_INVALID:
        default:
            printf("%s\n", ERR_UNDEFINED_COMMAND);
            break;
    }
    return COMMAND_CONTINUE; /* Continue */
}

/**
 * Main program entry point and command loop.
 * @return exit code
 */
int main(void) {
    char input_line[MAX_INPUT_BUFFER_SIZE];
    complex variables[NUM_COMPLEX_VARS]; /* A-F variables in main scope */

    /* Initialize all variables to zero */
    initialize_variables(variables);

    /* Main command processing loop */
    while (1) {
        InputReadResult read_result;

        /* Display prompt */
        printf("%s", PROMPT_ENTER_COMMAND);

        /* Read input line */
        read_result = read_input_line(input_line, sizeof(input_line));
        if (read_result == INPUT_EOF) {
            break; /* EOF */
        }
        if (read_result == INPUT_INVALID) {
            continue; /* Invalid input line, prompt again */
        }

        /* Process the command and check if we should exit */
        if (process_command(variables, input_line) == COMMAND_EXIT) {
            break; /* Exit requested */
        }
    }

    return 0;
}
