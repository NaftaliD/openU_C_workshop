# Makefile for Two-Pass Assembler
# Compiler: gcc with ANSI C90 compliance flags
# Project: Open University 20465 - Systems Programming Laboratory
# Auther: Naftali Deutsch

CC = gcc
CFLAGS = -pedantic -ansi -Wall -std=c90 -g
EXECUTABLE = assembler
OBJDIR = build_artifacts

# Core source files (macro pass only, for now)
SOURCES = assembler_core.c utils.c common.c macro_processor.c macro_table.c logger.c
HEADERS = common.h utils.h macro_processor.h macro_table.h logger.h

# Object files - kept out of the source tree in build_artifacts/; the
# executable itself stays at the top level since it's the deliverable.
OBJECTS = $(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))

# Default target
all: $(EXECUTABLE)

# Build executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS)
	@echo "Build completed: $(EXECUTABLE)"

# Compile .c files to .o files inside $(OBJDIR)
$(OBJDIR)/%.o: %.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean build artifacts
clean:
	-rm -rf $(OBJDIR)
	-rm -f $(EXECUTABLE) $(EXECUTABLE).exe
	@echo "Clean completed"

# Remove all generated files (including output files from test runs)
distclean: clean
	-rm -f *.am *.ob *.ent *.ext *.log
	@echo "Distclean completed"
