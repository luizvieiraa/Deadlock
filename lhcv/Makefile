CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

banker: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o banker

.PHONY: clean
clean:
	rm -f banker banker.exe *.o
