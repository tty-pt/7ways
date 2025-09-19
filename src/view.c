#include "../include/draw.h"
#include "../include/tile.h"
#include "../include/cam.h"
#include "../include/time.h"
#include "../include/char.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#include <qsys.h>
#include <qmap.h>
#include <geo.h>
#include <point.h>

double char_speed = 7.0;
double hw, hh;
extern uint8_t dim;
unsigned char_hd, map_hd, smap_hd;
cam_t cam;
img_t floors_img;
unsigned floors_tm;
double ww, wh;

uint8_t anim_frames[] = {
	[AN_WALK] = 0,
	[AN_IDLE] = 4,
};

void char_render(unsigned ref)
{
	const char_t *ch = qmap_get(char_hd, &ref);
	const tm_t *tm = tm_get(ch->tm_ref);
	uint8_t anim_n = anim_frames[AN_IDLE]
		? anim_frames[AN_IDLE]
		: tm->nx;
	uint32_t xn = ((int) (time_tick * char_speed))
		% anim_n;
	double offs = tm->w / 32.0;
	double inc = 16.0 * cam.zoom;
	uint32_t scr_x = hw + (ch->x + ch->nx - offs + 0.5 - cam.x) * inc,
		scr_y = hh + (ch->y + ch->ny - offs - cam.y) * inc;

	tm_render(ch->tm_ref, scr_x, scr_y, xn,
			ch->anim * 4 + ch->dir,
			cam.zoom * tm->w,
			cam.zoom * tm->h);
}

void
char_face(unsigned ref, enum dir dir)
{
	char_t *ch = (char_t *)
		qmap_get(char_hd, &ref);

	ch->dir = dir;
}

void
char_animate(unsigned ref, enum anim anim)
{
	char_t *ch = (char_t *)
		qmap_get(char_hd, &ref);

	ch->anim = anim;
}

enum dir
char_dir(unsigned ref)
{
	const char_t *ch = qmap_get(char_hd, &ref);
	return ch->dir;
}

enum anim
char_animation(unsigned ref)
{
	const char_t *ch = qmap_get(char_hd, &ref);
	return ch->anim;
}

void
char_pos(double *x, double *y, unsigned ref)
{
	const char_t *ch = qmap_get(char_hd, &ref);

	*x = ch->x + ch->nx;
	*y = ch->y + ch->ny;
}

void view_tl(int16_t tl[dim], uint8_t ow) {
	uint16_t n_ow = ow / 16;

	tl[0] = cam.x - ww / 2 - n_ow;
	tl[1] = cam.y - wh / 2 - n_ow;
	tl[2] = 0;
}

void view_len(uint16_t l[dim], uint8_t ow) {
	uint16_t n_ow = ow / 16;

	l[0] = ww + 1 + 2 * n_ow;
	l[1] = wh + 1 + 2 * n_ow;
	l[2] = 1;
}

unsigned view_iter(unsigned pdb_hd, uint8_t ow) {
	static int16_t quad_s[3];
	static uint16_t quad_l[3];

	view_tl(quad_s, ow);
	view_len(quad_l, ow);

	return geo_iter(pdb_hd, quad_s, quad_l, dim);
}

static inline void
tiles_render(void)
{
	unsigned ref;
	double inc = 16.0 * cam.zoom;
	int16_t p[dim];

	unsigned cur = view_iter(map_hd, 16);

	while (geo_next(p, &ref, cur))
		tile_render(ref, p);
}

static inline void
chars_render(void)
{
	unsigned ref;
	int16_t p[dim];

	unsigned cur = view_iter(smap_hd, 32);

	while (geo_next(p, &ref, cur))
		char_render(ref);
}

void
view_render(void) {
	tiles_render();
	chars_render();
}

static inline void
mymap_put(int16_t x, int16_t y, int16_t z, unsigned ref)
{
	int16_t p[] = { x, y, z };
	geo_put(map_hd, p, ref, dim);
}

unsigned
char_load(unsigned tm_ref, double x, double y) {
	char_t ch;
	unsigned ret;

	ch.tm_ref = tm_ref;
	ch.x = x;
	ch.y = y;
	ch.anim = AN_IDLE;
	ch.dir = DIR_DOWN;

	ret = qmap_put(char_hd, NULL, &ch);
	WARN("load char %u at %0.2lf %0.2lf: %u\n",
			ret, x, y, tm_ref);

	int16_t s[] = { x, y, 0 };

	geo_put(smap_hd, s, ret, dim);
	return ret;
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

		space = strchr(word, ' ');
		CBUG(!space, "file input end: B\n");
		*space = '\0';

		w = strtold(word, NULL);

		word = space + 1;
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

		space = strchr(word, ' ');
		CBUG(!space, "file input end: E\n");
		*space = '\0';

		tm_ref = strtold(word, NULL);

		word = space + 1;
		space = strchr(word, ' ');
		CBUG(!space, "file input end: F\n");
		*space = '\0';

		tm_x = strtold(word, NULL);

		word = space + 1;
		space = strchr(word, '\n');
		CBUG(!space, "file input end: G\n");
		*space = '\0';

		tm_y = strtold(word, NULL);

		tile_add(tm_ref, tm_x, tm_y);
	}

	uint16_t w, h, d,
		 mhw, mhh;

	ret = fgets(line, sizeof(line), fp);
	word = line;

	CBUG(!ret || *line == '\n', "file input end: H\n");

	space = strchr(word, ' ');
	CBUG(!space, "file input end: I\n");
	*space = '\0';

	w = strtold(word, NULL);
	mhw = w / 2;

	word = space + 1;
	space = strchr(word, ' ');
	CBUG(!space, "file input end: J\n");
	*space = '\0';

	h = strtold(word, NULL);
	mhh = h / 2;

	word = space + 1;
	space = strchr(word, '\n');
	CBUG(!space, "file input end: K\n");
	*space = '\0';

	d = strtold(word, NULL);

	fprintf(stderr, "dim %d %d %d\n", w, h, d);

	ret = fgets(line, sizeof(line), fp);
	CBUG(!ret, "file input end: L\n");

	for (uint16_t id = 0; id < d; id ++) {
		fprintf(stderr, "layer %u\n", id);
		for (uint16_t ih = 0; ih < h; ih ++)
		{
			char *s = line;
			ret = fgets(line, sizeof(line), fp);
			CBUG(!ret, "file input end: M\n");
			fprintf(stderr, "line %s\n", line);

			for (uint16_t iw = 0; iw < w; iw ++, s++)
			{
				unsigned stile_ref = (unsigned) (*s - 'b');
				fprintf(stderr, "put %u %d %d %d\n",
						stile_ref, iw, ih, id);
				mymap_put(iw - mhw, ih - mhh,
						id, stile_ref);
				n++;
			}
		}
	}

	fprintf(stderr, "loaded %u tiles\n", n);
}

void
view_init(void)
{
	unsigned qm_char = qmap_reg(sizeof(char_t));

	cam.x = 0;
	cam.y = 0;
	cam.zoom = 8;

	hw = 0.5 * ((double) be_width) - 8.0 * cam.zoom;
	hh = 0.5 * ((double) be_height) - 8.0 * cam.zoom;
	ww = be_width / 16.0 / cam.zoom;
	wh = be_height / 16.0 / cam.zoom;

	char_hd = qmap_open(QM_HNDL, qm_char,
			0xFF, QM_AINDEX);

	geo_init();
	map_hd = geo_open("map", 0x1FFF);
	smap_hd = geo_open("smap", 0x1FFF);

	view_load("./map.txt");

	unsigned rooster_img = img_load("./resources/rooster.png");
	unsigned rooster_tm = tm_load(rooster_img, 32, 32);
	char_load(rooster_tm, 2, 1);
}

static inline void
char_update(unsigned ref, double dt)
{
	char_t *ch = (char_t *) qmap_get(char_hd, &ref);
	char_t cho;
	double char_speed = 4.0, tr;

	if (ch->anim == AN_IDLE)
		return;

	tr = dt * char_speed;

	int16_t p[] = { ch->x, ch->y, 0 };

	switch (ch->dir) {
		case DIR_UP:
			ch->ny -= tr;
			break;
		case DIR_DOWN:
			ch->ny += tr;
			break;
		case DIR_LEFT:
			ch->nx -= tr;
			break;
		case DIR_RIGHT:
			ch->nx += tr;
			break;
	}

	if (fabs(ch->nx) < 1.0 && fabs(ch->ny) < 1.0)
		return;

	ch->x = round(ch->x + ch->nx);
	ch->y = round(ch->y + ch->ny);
	ch->nx = ch->ny = 0;
	ch->anim = AN_IDLE;
	cho = *ch;

	geo_del(smap_hd, p, dim);
	p[0] = ch->x;
	p[1] = ch->y;
	qmap_del(char_hd, &ref);
	qmap_put(char_hd, &ref, &cho);
	geo_put(smap_hd, p, ref, dim);

	/* WARN("update! %u %0.2lf\n", ref, dt); */
}

void
view_update(double dt)
{
	unsigned cur = qmap_iter(smap_hd, NULL, 0);
	const void *key, *value;

	while (qmap_next(&key, &value, cur))
		char_update(* (unsigned *) value, dt);
}
