#ifndef TILE_H
#define TILE_H

#include <stdint.h>

typedef struct {
	unsigned img;
	uint32_t w, h, nx, ny;
} tm_t;

extern double sprite_speed;

void tile_init(void);

unsigned tm_load(unsigned img_ref, uint32_t w, uint32_t h);
const tm_t *tm_get(unsigned tm_ref);

void tm_render(unsigned ref,
		uint32_t x, uint32_t y,
		uint32_t nx, uint32_t ny,
		uint32_t w, uint32_t h,
		uint32_t rx, uint32_t ry);

unsigned tile_add(unsigned tm_ref, uint16_t x, uint16_t y);
void tile_render(unsigned tm_ref, unsigned idx, int16_t *p);

#endif
