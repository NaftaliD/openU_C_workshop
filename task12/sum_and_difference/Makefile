CC = gcc
CFLAGS = -pedantic -ansi -Wall -std=c90
TARGET = sum_and_difference

all: $(TARGET)

$(TARGET): sum_and_difference.c
	$(CC) $(CFLAGS) -o $(TARGET) sum_and_difference.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
