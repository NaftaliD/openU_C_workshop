CC = gcc
CFLAGS = -pedantic -ansi -Wall -std=c90 -g 
LDFLAGS = -lm
EXECUTABLE = mycomp

SOURCES = mycomp.c complex.c common.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(EXECUTABLE)

# Build executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LDFLAGS)
	@echo "Build completed: $(EXECUTABLE)"

# Compile .c files to .o files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	-$(RM) $(OBJECTS) $(EXECUTABLE)
	@echo "Clean completed"