CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

banker: main.c input.c input.h
	$(CC) $(CFLAGS) main.c input.c -o banker

.PHONY: clean
clean:
	rm -f banker banker.exe *.o
