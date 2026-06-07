CC = gcc
CFLAGS = -pedantic -ansi -Wall -std=c90
EXECUTABLE = sum_and_difference

SOURCES = sum_and_difference.c

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
	-$(RM) $(OBJECTS) $(EXECUTABLE)
	@echo "Clean completed"