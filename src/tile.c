#include "../include/tile.h"
#include "../include/cam.h"
#include "../include/view.h"
#include "../include/font.h"

#include <stdio.h>
#include <string.h>

#include <ttypt/qmap.h>
#include <ttypt/qsys.h>

typedef struct {
	unsigned tm_ref,
		 tm_x, tm_y,
		 flags;
} stile_t;

extern cam_t cam;

static unsigned tm_hd, stile_hd;

unsigned
tm_load(unsigned img_ref, uint32_t w, uint32_t h)
{
	tm_t tm;
	uint32_t img_w, img_h;

	tm.img = img_ref;
	img_size(&img_w, &img_h, img_ref);
	tm.w = w;
	tm.h = h;
	tm.nx = img_w / w;
	tm.ny = img_h / h;

	unsigned ref = qmap_put(tm_hd, NULL, &tm);
	WARN("tm_load %u: %u %u %u\n", ref, img_ref,
			w, h);
	return ref;
}

void
tm_render(unsigned ref, uint32_t x, uint32_t y,
		uint32_t nx, uint32_t ny,
		uint32_t w, uint32_t h,
		uint32_t rx, uint32_t ry)
{
	const tm_t *tm = qmap_get(tm_hd, &ref);

	if (!w)
		w = tm->w;
	if (!h)
		h = tm->h;

	for (uint32_t iy = 0; iy < ry * h; iy += h)
		for (uint32_t ix = 0; ix < rx * w; ix += w)
			img_render_ex(tm->img,
					x + ix,
					y + iy,
					nx * tm->w,
					ny * tm->h,
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
tile_render(unsigned tm_ref, unsigned idx, int16_t *p)
{
	char idx_buf[BUFSIZ];
	unsigned x = view_hw + (p[0] - cam.x) * view_mul;
	unsigned y = view_hh + (p[1] - cam.y) * view_mul;
	const tm_t *tm = tm_get(tm_ref);
	const tm_t *tm_font = tm_get(font_ref);
	unsigned tm_x = idx % tm->nx;
	unsigned tm_y = idx / tm->nx;

	tm_render(tm_ref,
			x, y,
			tm_x, tm_y,
			view_mul, view_mul,
			1, 1);

	sprintf(idx_buf, "%u", idx);
	/* font_render(font_ref, idx_buf, */
	/* 		x, y, */
	/* 		x + tm_font->w * strlen(idx_buf), */
	/* 		y + tm_font->h, */
	/* 		1); */
}
