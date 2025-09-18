#include "../include/tile.h"

#include <qmap.h>

/*
typedef struct {
	unsigned img;
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

typedef struct {
	tm_t tm;
	uint32_t n;
	double speed;
} sprite_t;
*/

unsigned tm_hd;

unsigned
tm_load(unsigned img_ref, uint32_t w, uint32_t h)
{
	tm_t tm;
	const img_t *img;

	tm.img = img_ref;
	img = img_get(img_ref);
	tm.w = w;
	tm.h = h;
	tm.nx = img->w / w;
	tm.ny = img->h / h;

	return qmap_put(tm_hd, NULL, &tm);
}

void
tm_render(unsigned ref, uint32_t x, uint32_t y,
		uint32_t nx, uint32_t ny,
		uint32_t w, uint32_t h)
{
	const tm_t *tm = qmap_get(tm_hd, &ref);

	if (!w)
		w = tm->w;
	if (!h)
		h = tm->h;

	img_render(tm->img, x, y,
			nx * tm->w, ny * tm->h,
			tm->w, tm->h,
			w, h);
}

const tm_t *
tm_get(unsigned ref)
{
	return qmap_get(tm_hd, &ref);
}

void
tm_init(void)
{
	unsigned qm_tm = qmap_reg(sizeof(tm_t));
	tm_hd = qmap_open(QM_HNDL, qm_tm, 0xF, QM_AINDEX);
}
