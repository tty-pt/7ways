#ifndef TILE_H
#define TILE_H

#include "./img.h"

typedef struct {
	img_t *img;
	uint32_t w, h, nx, ny;
} tm_t;

typedef struct {
	tm_t *tm;
	unsigned tm_x, tm_y, flags;
} stile_t;

typedef struct {
	unsigned sk;
	int16_t x, y;
} tile_t;

static inline tm_t
tm_load(img_t *img, uint32_t w, uint32_t h)
{
	tm_t tm;

	tm.img = img;
	tm.w = w;
	tm.h = h;
	tm.nx = tm.img->w / w;
	tm.ny = tm.img->h / h;

	return tm;
}

static inline void
tm_render(tm_t *tm, uint32_t x, uint32_t y,
		uint32_t nx, uint32_t ny,
		uint32_t w, uint32_t h)
{
	if (!w)
		w = tm->w;
	if (!h)
		h = tm->h;

	img_render(tm->img, x, y,
			nx * tm->w, ny * tm->h,
			tm->w, tm->h,
			w, h);
}

#endif
