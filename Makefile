CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

banker: lhcv/banker.c
	$(CC) $(CFLAGS) lhcv/banker.c -o banker

.PHONY: clean
clean:
	rm -f banker banker.exe *.o
