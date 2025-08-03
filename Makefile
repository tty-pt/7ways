CC = gcc
CFLAGS = -Wall -Wextra -O3
SRC = simple.c simple16.c simple32.c anime.c anime_fast.c anime_fastest.c anime_port.c
EXEC = simple simple16 simple32 anime anime_fast anime_fastest anime_bun anime_port

all: $(EXEC)

simple: simple.c
	$(CC) $(CFLAGS) -o $@ $^

simple16: simple16.c
	$(CC) $(CFLAGS) -o $@ $^

simple32: simple32.c
	$(CC) $(CFLAGS) -o $@ $^

anime: anime.c
	$(CC) $(CFLAGS) -o $@ $^

anime_fast: anime_fast.c
	$(CC) $(CFLAGS) -o $@ $^

anime_fastest: anime_fastest.c
	$(CC) $(CFLAGS) -o anime_fastest anime_fastest.c -lm

anime_bun:
	bun build --production --compile --outfile=anime_bun anime.mjs

anime_port: anime_port.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -f $(EXEC)

.PHONY: all clean