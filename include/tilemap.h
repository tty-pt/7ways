#ifndef TILEMAP_H
#define TILEMAP_H

#include "./img.h"

typedef struct {
	img_t img;
	uint32_t w, h, nx, ny;
} tm_t;

tm_t tm_load(char *filename, uint32_t w, uint32_t h);

static inline void
tm_render(tm_t *tm, uint32_t x, uint32_t y,
		uint32_t nx, uint32_t ny)
{
	img_render(&tm->img, x, y,
			nx * tm->w, ny * tm->h,
			tm->w, tm->h);
}

#endif
