/*
 * assembler_core.h
 * Entry point and per-file pipeline orchestration.
 */

#ifndef ASSEMBLER_CORE_H
#define ASSEMBLER_CORE_H

/**
 * Processes each given .as file independently through the full assembler
 * pipeline (macro expansion, Pass One, Pass Two). An error in one file does
 * not stop the others from being processed.
 * @param argc number of command-line arguments (including program name)
 * @param argv argument vector: argv[1..] are .as file paths
 * @return 0 on completion, 1 if no input files were given
 */
int runAssembler(int argc, char *argv[]);

#endif /* ASSEMBLER_CORE_H */
