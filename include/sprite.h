#ifndef SPRITE_H
#define SPRITE_H

#include "./tile.h"

extern double time_tick;

void sprite_render(sprite_t *sprite,
		uint32_t x, uint32_t y,
		uint32_t sx, uint32_t sy);

#endif
