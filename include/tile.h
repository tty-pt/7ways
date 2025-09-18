#ifndef TILE_H
#define TILE_H

#include "./img.h"

typedef struct {
	unsigned img;
	uint32_t w, h, nx, ny;
} tm_t;

typedef struct {
	unsigned tm_ref,
		 tm_x, tm_y,
		 flags;
} stile_t;

typedef struct {
	unsigned sk;
	int16_t x, y;
} tile_t;

extern double sprite_speed;

void tm_init(void);
unsigned tm_load(unsigned img_ref, uint32_t w, uint32_t h);
const tm_t *tm_get(unsigned tm_ref);

void tm_render(unsigned ref,
		uint32_t x, uint32_t y,
		uint32_t nx, uint32_t ny,
		uint32_t w, uint32_t h);

#endif
