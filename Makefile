CC = gcc
CFLAGS = -Wall -Wextra -O3
C_SRC = C/anime.c C/static.c
EXEC = anime static anime_bun

all: $(EXEC)

anime: C/anime.c
	$(CC) $(CFLAGS) -o $@ $< -lm

static: C/static.c
	$(CC) $(CFLAGS) -o $@ $<

anime_bun:
	bun build --production --compile --outfile=anime_bun JS/anime.mjs

clean:
	rm -f anime static anime_bun

.PHONY: all clean
