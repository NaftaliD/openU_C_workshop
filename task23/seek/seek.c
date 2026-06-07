#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* ── Error messages ─────────────────────────────────────────────────────── */
#define ERR_TOO_FEW_ARGS  "Error: at least 2 arguments required (n and at least one input file)"
#define ERR_INVALID_N     "Error: first argument must be a positive integer (greater than 0)"
#define ERR_CANNOT_OPEN   "Error: cannot open file '%s'"
#define ERR_FILE_SHORT    "Error: file '%s' has fewer than %ld characters"

/* ── Output format ──────────────────────────────────────────────────────── */
#define OUTPUT_FORMAT     "File '%s': character at position %ld has ASCII code %d"

/* ── Argument count threshold ───────────────────────────────────────────── */
#define MIN_ARGC 3  /* argv[0]=program, argv[1]=n, argv[2..]=files */


/**
 * Parses a string as a strictly positive integer.
 * Uses strtol with full end-pointer validation to reject partial strings,
 * non-numeric input, zero, and negative values.
 * @param str  the string to parse (from argv)
 * @param out  receives the parsed value on success
 * @return 1 if str is a valid positive integer, 0 otherwise
 */
static int parse_n(const char *str, long *out)
{
    char *endptr;
    long value;

    errno = 0;
    value = strtol(str, &endptr, 10);

    /* Reject: no digits consumed, trailing non-digit chars, overflow, <= 0 */
    if (endptr == str || *endptr != '\0' || errno == ERANGE || value <= 0) {
        return 0;
    }

    *out = value;
    return 1;
}

/**
 * Seeks to the nth character (1-indexed) in the given file and prints
 * its ASCII code in decimal to stdout.
 * Uses fseek to navigate directly — no sequential character reading.
 * Prints a per-file error and returns (without stopping) if the file
 * cannot be opened or contains fewer than n characters.
 * @param path  file system path to the input file
 * @param n     1-indexed position of the target character (offset = n-1)
 */
static void process_file(const char *path, long n)
{
    FILE *f;
    int ch;

    f = fopen(path, "rb");
    if (f == NULL) {
        printf(ERR_CANNOT_OPEN "\n", path);
        return;
    }

    /* fseek to byte offset n-1 from the start of the file */
    if (fseek(f, n - 1, SEEK_SET) != 0) {
        printf(ERR_FILE_SHORT "\n", path, n);
        fclose(f);
        return;
    }

    ch = fgetc(f);
    if (ch == EOF) {
        /* fseek succeeded but position is past end-of-file */
        printf(ERR_FILE_SHORT "\n", path, n);
        fclose(f);
        return;
    }

    printf(OUTPUT_FORMAT "\n", path, n, ch);
    fclose(f);
}

/**
 * Program entry point.
 * Validates that at least 2 arguments were supplied and that the first
 * is a positive integer n, then processes each remaining argument as an
 * input file path.
 * Fatal errors (bad argc or invalid n) terminate the program immediately.
 * Per-file errors print a message and continue to the next file.
 * @param argc  number of command-line arguments (including program name)
 * @param argv  argument vector
 * @return 0 on normal completion, 1 on fatal argument error
 */
int main(int argc, char *argv[])
{
    long n;
    int i;

    if (argc < MIN_ARGC) {
        printf("%s\n", ERR_TOO_FEW_ARGS);
        return 1;
    }

    if (!parse_n(argv[1], &n)) {
        printf("%s\n", ERR_INVALID_N);
        return 1;
    }

    for (i = 2; i < argc; i++) {
        process_file(argv[i], n);
    }

    return 0;
}
