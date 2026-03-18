CC = gcc
CFLAGS = -pedantic -ansi -Wall -std=c90
TARGET = on_turn

all: $(TARGET)

$(TARGET): on_turn.c
	$(CC) $(CFLAGS) -o $(TARGET) on_turn.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
