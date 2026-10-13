CC ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic

main: main.c
	$(CC) $(CFLAGS) $< -o $@

test: main
	python3 -B -m unittest discover -s tests -v

sanitize:
	mkdir -p .build
	$(CC) -std=c99 -O1 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer main.c -o .build/main-sanitize
	PROGRAM=$(CURDIR)/.build/main-sanitize python3 -B -m unittest discover -s tests -v

clean:
	rm -f main
	rm -f .build/main-sanitize

.PHONY: test sanitize clean
