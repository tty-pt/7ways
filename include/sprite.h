#ifndef SPRITE_H
#define SPRITE_H

#include "./tm.h"

typedef struct {
	tm_t tm;
	uint32_t n;
	double speed;
} sprite_t;

extern double t;

static inline void sprite_render(sprite_t *sprite,
		uint32_t x, uint32_t y,
		uint32_t sx, uint32_t sy)
{
	uint32_t xn = ((int) (t * sprite->speed))
		% sprite->tm.nx;

	tm_render(&sprite->tm, x, y, xn, sprite->n, sx, sy);
}

#endif
