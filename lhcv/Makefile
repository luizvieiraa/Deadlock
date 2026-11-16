CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

banker: banker.c
	$(CC) $(CFLAGS) banker.c -o banker

.PHONY: clean
clean:
	rm -f banker banker.exe *.o
