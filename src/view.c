#include "../include/draw.h"
#include "../include/tile.h"
#include "../include/cam.h"
#include "../include/char.h"
#include "../include/dialog.h"

#include <string.h>
#include <stdio.h>

#include <qsys.h>
#include <qmap.h>
#include <geo.h>
#include <point.h>

typedef struct {
	unsigned bm_ref, tm_ref;
} layer_t;

static unsigned me = 0;
extern uint8_t dim;
unsigned smap_hd;
cam_t cam;
unsigned floors_tm;
double view_mul,
       view_w, view_h,
       view_hw, view_hh;

layer_t view_layers[2];

int16_t view_min[3], view_max[3];

void view_tl(int16_t tl[dim], uint8_t ow) {
	uint16_t n_ow = ow / 16;

	tl[0] = cam.x - view_w / 2 - n_ow;
	tl[1] = cam.y - view_h / 2 - n_ow;
	tl[2] = 0;
}

void view_len(uint16_t l[dim], uint8_t ow) {
	uint16_t n_ow = ow / 16;

	l[0] = view_w + 2 + 2 * n_ow;
	l[1] = view_h + 2 + 2 * n_ow;
	l[2] = 4;
}

// idx: 0..255  ->  RGBA (R:3 bits, G:3 bits, B:2 bits)
static inline uint32_t view_col(uint8_t idx)
{
	// extract intertwined bits:
	// R0,G0,B0,R1,G1,B1,R2,G2

	unsigned r3 = ((idx >> 0) & 1u)
		| (((idx >> 3) & 1u) << 1)
		| (((idx >> 6) & 1u) << 2);

	unsigned g3 = ((idx >> 1) & 1u)
		| (((idx >> 4) & 1u) << 1)
		| (((idx >> 7) & 1u) << 2);

	unsigned b2 = ((idx >> 2) & 1u)
		| (((idx >> 5) & 1u) << 1);

	// scale 3/3/2 bits to 8 bits with rounding
	uint8_t r = (uint8_t)((r3 * 255u + 3u) / 7u);
	uint8_t g = (uint8_t)((g3 * 255u + 3u) / 7u);
	uint8_t b = (uint8_t)((b2 * 255u + 2u) / 3u);

	return 0xFF000000u
		| ((uint32_t) r << 16)
		| ((uint32_t) g << 8)
		| b;
}

static inline unsigned view_idx(uint32_t col)
{
    uint8_t r8 = (col >> 16) & 0xFF;
    uint8_t g8 = (col >>  8) & 0xFF;
    uint8_t b8 = (col      ) & 0xFF;

    // quantize
    unsigned r3 = (unsigned)((r8 * 7u + 127u) / 255u);
    unsigned g3 = (unsigned)((g8 * 7u + 127u) / 255u);
    unsigned b2 = (unsigned)((b8 * 3u + 127u) / 255u);

    // intertwine
    return  ((r3 & 1u) << 0)
          | ((g3 & 1u) << 1)
          | ((b2 & 1u) << 2)
          | (((r3 >> 1) & 1u) << 3)
          | (((g3 >> 1) & 1u) << 4)
          | (((b2 >> 1) & 1u) << 5)
          | (((r3 >> 2) & 1u) << 6)
          | (((g3 >> 2) & 1u) << 7);
}

static inline void
view_bmtl(uint32_t *target, int16_t *tl)
{
	target[0] = tl[0] - view_min[0];
	target[1] = tl[1] - view_min[1];
}

static inline void
view_render_bm(int16_t *tl, uint32_t *bmtl, uint16_t *len,
		unsigned layer_idx)
{
	layer_t *layer = &view_layers[layer_idx];

	for (uint32_t y = 0; y < len[1]; y++)
		for (uint32_t x = 0; x < len[0]; x++)
		{
			int16_t p[] = { x + tl[0], y + tl[1], layer_idx };
			uint32_t col = img_pick(layer->bm_ref,
					x + bmtl[0],
					y + bmtl[1]);

			unsigned idx = view_idx(col);
			tile_render(layer->tm_ref, idx, p);
		}
}

void
view_render(void) {
	unsigned ref;
	int16_t p[dim];
	unsigned cur;
	uint16_t l[dim];
	int16_t tl[dim];
	uint32_t bmtl[dim];

	view_tl(tl, 16);
	view_bmtl(bmtl, tl);
	view_len(l, 16);

	view_render_bm(tl, bmtl, l, 0);
	view_render_bm(tl, bmtl, l, 1);

	cur = geo_iter(smap_hd, tl, l, dim);
	while (geo_next(p, &ref, cur))
		char_render(ref);
}

void
vchar_put(unsigned ref, int16_t x, int16_t y)
{
	int16_t s[] = { x, y, 0, 0 };
	geo_put(smap_hd, s, ref, dim);
}

void
view_load(char *filename) {
	char line[BUFSIZ];
	FILE *fp = fopen("./map.txt", "r");
	char *space, *word, *ret;

	CBUG(!fp, "couldn't open %s\n", filename);

	while (1) {
		uint16_t w;

		ret = fgets(line, sizeof(line), fp);
		word = line;

		CBUG(!ret, "file input end: A\n");

		if (*line == '\n')
			break;

		w = strtold(word, &word);

		word++;
		space = strchr(word, '\n');
		CBUG(!space, "file input end: C\n");
		*space = '\0';

		unsigned img = img_load(word);
		tm_load(img, w, w);
	}

#if !CHAR_SYNC
	while (fgets(line, sizeof(line), fp)) {
		unsigned ref;
		uint16_t x, y;

		word = line;

		ref = strtold(word, &word);
		x = strtold(word, &word);
		y = strtold(word, NULL);

		char_load(ref, x, y);
	}
#endif

	uint32_t w, h;
	view_layers[0].bm_ref = img_load("./map/0.png");
	view_layers[1].bm_ref = img_load("./map/1.png");
	img_size(&w, &h, view_layers[0].bm_ref);
	view_min[0] = -w / 2;
	view_min[1] = -h / 2;
	view_min[2] = 0;

	point_debug("view_min", view_min, dim);
}

void
view_init(void)
{
	geo_init();

	cam.x = 0;
	cam.y = 0;
	cam.zoom = 8;
	view_mul = 16.0 * cam.zoom;
	view_hw = 0.5 * ((double) be_width - view_mul);
	view_hh = 0.5 * ((double) be_height - view_mul);
	view_w = be_width / view_mul;
	view_h = be_height / view_mul;

	smap_hd = geo_open("smap", 0xFFFFF);
	qmap_drop(smap_hd);
}

static inline void
vchar_update(unsigned ref, double dt)
{
	int16_t p[] = { 0, 0, 0, 0 };

	char_ipos(p, ref);

	if (char_update(ref, dt))
		return;

	geo_del(smap_hd, p, dim);
	char_ipos(p, ref);
	geo_put(smap_hd, p, ref, dim);
}

void
view_paint(unsigned ref, unsigned tile, uint16_t layer)
{
	double x, y;

	char_pos(&x, &y, ref);
	img_paint(view_layers[layer].bm_ref,
			((int16_t) x) - view_min[0],
			((int16_t) y) - view_min[1],
			view_col(tile));
}

unsigned
view_collides(double x, double y, enum dir dir)
{
	switch (dir) {
		case DIR_UP:
			y -= 1;
			break;
		case DIR_DOWN:
			y += 1;
			break;
		case DIR_LEFT:
			x -= 1;
			break;
		case DIR_RIGHT:
			x += 1;
	}

	int16_t p[] = { x, y, 0, 0 };

	return geo_get(smap_hd, p, dim);
}

void
view_update(double dt)
{
	unsigned cur = qmap_iter(smap_hd, NULL, 0);
	const void *key, *value;

	while (qmap_next(&key, &value, cur))
		vchar_update(* (unsigned *) value, dt);
}

void
view_sync(void)
{
	img_save(view_layers[0].bm_ref);
	img_save(view_layers[1].bm_ref);
}

int
vdialog_action(void)
{
	double x, y;
	unsigned npc;
	enum dir dir = char_dir(me);

	if (dialog_action())
		return 1;

	char_pos(&x, &y, me);
	npc = view_collides(x, y, dir);

	if (npc == QM_MISS)
		return 0;

	char_talk(npc, dir);
	return 1;
}
