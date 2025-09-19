#include "../include/tile.h"
#include "../include/cam.h"

#include <qmap.h>

typedef struct {
	unsigned tm_ref,
		 tm_x, tm_y,
		 flags;
} stile_t;

/*
typedef struct {
	unsigned img;
	uint32_t w, h, nx, ny;
} tm_t;

typedef struct {
	tm_t tm;
	uint32_t n;
	double speed;
} sprite_t;
*/

extern double hw, hh;
extern cam_t cam;

static double inc;
static unsigned tm_hd, stile_hd;

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
tile_init(void)
{
	unsigned qm_tm = qmap_reg(sizeof(tm_t));
	unsigned qm_stile = qmap_reg(sizeof(stile_t));

	tm_hd = qmap_open(QM_HNDL, qm_tm, 0xF, QM_AINDEX);
	stile_hd = qmap_open(QM_HNDL, qm_stile,
			0xFF, QM_AINDEX);

	inc = 16.0 * cam.zoom;
}

unsigned
tile_add(unsigned tm_ref, uint16_t x, uint16_t y)
{
	stile_t stile;

	stile.tm_ref = tm_ref;
	stile.tm_x = x;
	stile.tm_y = y;

	return qmap_put(stile_hd, NULL, &stile);
}

void
tile_render(unsigned ref, int16_t *p)
{
	const stile_t *stile
		= qmap_get(stile_hd, &ref);
	double inc = 16.0 * cam.zoom;

	tm_render(stile->tm_ref,
			hw + (p[0] - cam.x) * inc,
			hh + (p[1] - cam.y) * inc,
			stile->tm_x,
			stile->tm_y,
			16.0 * cam.zoom,
			16.0 * cam.zoom);
}
