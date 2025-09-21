#include "../include/draw.h"
#include "../include/tile.h"
#include "../include/cam.h"
#include "../include/char.h"
#include "../include/dialog.h"

#include <string.h>
#include <stdio.h>

#include <qsys.h>
#include <qdb.h>
#include <geo.h>
#include <point.h>

static unsigned me = 0;
extern uint8_t dim;
unsigned map_hd, smap_hd;
cam_t cam;
img_t floors_img;
unsigned floors_tm;
double view_mul,
       view_w, view_h,
       view_hw, view_hh;

void view_tl(int16_t tl[dim], uint8_t ow) {
	uint16_t n_ow = ow / 16;

	tl[0] = cam.x - view_w / 2 - n_ow;
	tl[1] = cam.y - view_h / 2 - n_ow;
	tl[2] = 0;
}

void view_len(uint16_t l[dim], uint8_t ow) {
	uint16_t n_ow = ow / 16;

	l[0] = view_w + 1 + 2 * n_ow;
	l[1] = view_h + 1 + 2 * n_ow;
	l[2] = 1;
}

unsigned view_iter(unsigned pdb_hd, uint8_t ow) {
	static int16_t quad_s[3];
	static uint16_t quad_l[3];

	view_tl(quad_s, ow);
	view_len(quad_l, ow);

	return geo_iter(pdb_hd, quad_s, quad_l, dim);
}

void
view_render(void) {
	unsigned ref;
	int16_t p[dim];
	unsigned cur;

	cur = view_iter(map_hd, 16);
	while (geo_next(p, &ref, cur))
		tile_render(ref, p);

	cur = view_iter(smap_hd, 32);
	while (geo_next(p, &ref, cur))
		char_render(ref);
}

static inline void
mymap_put(int16_t x, int16_t y, int16_t z, unsigned ref)
{
	int16_t p[] = { x, y, z };
	geo_put(map_hd, p, ref, dim);
}

unsigned
vchar_load(unsigned tm_ref, int16_t x, int16_t y) {
	unsigned ref = char_load(tm_ref, x, y);
	int16_t s[] = { x, y, 0 };

	geo_put(smap_hd, s, ref, dim);
	WARN("vchar_load %u: %d %d\n", ref, x, y);
	return ref;
}

void
view_load(char *filename) {
	char line[BUFSIZ];
	FILE *fp = fopen("./map.txt", "r");
	char *space, *word, *ret;
	unsigned n = 0;

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

	while (1) {
		int ch = 1;
		unsigned tm_ref;
		uint16_t tm_x, tm_y;

		ret = fgets(line, sizeof(line), fp);
		word = line;

		CBUG(!ret, "file input end: D\n");

		if (*line == '\n')
			break;

		tm_ref = strtold(word, &word);
		tm_x = strtold(word, &word);
		tm_y = strtold(word, &word);
		tile_add(tm_ref, tm_x, tm_y);
	}

	while (fgets(line, sizeof(line), fp)) {
		unsigned ref;
		uint16_t x, y;

		word = line;

		ref = strtold(word, &word);
		x = strtold(word, &word);
		y = strtold(word, NULL);

		vchar_load(ref, x, y);
	}
}

void
view_init(void)
{
	geo_init();

	dim = 3;
	cam.x = 0;
	cam.y = 0;
	cam.zoom = 8;
	view_mul = 16.0 * cam.zoom;
	view_hw = 0.5 * ((double) be_width - view_mul);
	view_hh = 0.5 * ((double) be_height - view_mul);
	view_w = be_width / view_mul;
	view_h = be_height / view_mul;

	map_hd = geo_open("map", 0x1FFF);
	smap_hd = geo_open("smap", 0x1FFF);
	qmap_drop(smap_hd);
}

static inline void
vchar_update(unsigned ref, double dt)
{
	int16_t p[] = { 0, 0, 0 };

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
	int16_t p[] = { 0, 0, 0 };
	char_ipos(p, ref);
	p[2] = layer;

	geo_del(map_hd, p, dim);
	char_ipos(p, ref);
	geo_put(map_hd, p, tile, dim);
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

	int16_t p[] = { x, y, 0 };

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
	qdb_sync(map_hd);
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
