.PHONY: build
build: decode

.PHONY: clean
clean:
	rm -f decode decode.debug

decode: decode.c
	gcc -std=gnu11 -D_GNU_SOURCE -O2 -s -o $@ $<

decode.debug: decode.c
	gcc -std=gnu11 -D_GNU_SOURCE -Og -g -Wall -Wextra -Werror -o $@ $<
