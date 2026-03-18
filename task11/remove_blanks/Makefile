CC = gcc
CFLAGS = -pedantic -ansi -Wall -std=c90
TARGET = remove_blanks

all: $(TARGET)

$(TARGET): remove_blanks.c
	$(CC) $(CFLAGS) -o $(TARGET) remove_blanks.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
