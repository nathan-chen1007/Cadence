CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined

cadence: src/*.c src/*.h
	$(CC) $(CFLAGS) -o cadence src/*.c

clean:
	rm -f cadence

.PHONY: clean