# Makefile for Two-Pass Assembler
# Compiler: gcc with ANSI C90 compliance flags
# Project: Open University 20465 - Systems Programming Laboratory
# Auther: Naftali Deutsch

CC = gcc
CFLAGS = -pedantic -ansi -Wall -std=c90 -g
EXECUTABLE = assembler

# Detect OS and set appropriate commands
ifeq ($(OS),Windows_NT)
    # Windows - use del
    RM = del /Q
    EXE_EXT = .exe
else
    # Linux/Unix/macOS - use rm
    RM = rm -f
    EXE_EXT = 
endif

# Core source files (macro pass only, for now)
SOURCES = assembler_core.c utils.c common.c macro_processor.c macro_table.c logger.c
HEADERS = common.h utils.h macro_processor.h macro_table.h logger.h

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(EXECUTABLE)

# Build executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS)
	@echo "Build completed: $(EXECUTABLE)"

# Compile .c files to .o files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	-$(RM) $(OBJECTS) $(EXECUTABLE)$(EXE_EXT)
	@echo "Clean completed"

# Remove all generated files (including output files from test runs)
distclean: clean
	-$(RM) *.am *.ob *.ent *.ext *.log
	@echo "Distclean completed"
