CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined

cadence: src/*.c src/*.h
	$(CC) $(CFLAGS) -o cadence src/*.c

test_dca: tests/test_dca.c src/*.c src/*.h
	$(CC) $(CFLAGS) -Isrc -o test_dca tests/test_dca.c $(filter-out src/main.c,$(wildcard src/*.c)) -lm

test: test_dca
	./test_dca

clean:
	rm -f cadence test_dca

.PHONY: test clean
