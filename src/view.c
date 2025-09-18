#include "../include/draw.h"
#include "../include/tile.h"
#include "../include/cam.h"
#include "../include/time.h"

#include <string.h>
#include <stdio.h>

#include <qsys.h>
#include <qmap.h>
#include <geo.h>
#include <point.h>

double hw, hh;
extern uint8_t dim;
unsigned stile_hd, map_hd;
cam_t cam;
img_t floors_img;
unsigned floors_tm;

void sprite_render(sprite_t *sprite,
		uint32_t x, uint32_t y,
		uint32_t sx, uint32_t sy)
{
	const tm_t *tm = tm_get(sprite->tm_ref);
	uint32_t xn = ((int) (time_tick * sprite->speed))
		% tm->nx;

	tm_render(sprite->tm_ref, x, y, xn, sprite->n, sx, sy);
}

void
view_render(void)
{
	// how many tiles in the view
	uint16_t w = be_width / 16.0 / cam.zoom,
		h = be_height / 16.0 / cam.zoom;

	unsigned ref;
	int16_t quad_s[dim], p[dim];
	uint16_t quad_l[dim];
	double inc = 16.0 * cam.zoom;

	quad_s[0] = cam.x;
	quad_s[1] = cam.y;
	quad_s[2] = 0;

	quad_l[0] = w + 1;
	quad_l[1] = h + 1;
	quad_l[2] = 1;

	unsigned cur = geo_iter(map_hd, quad_s, quad_l, dim);

	while (geo_next(p, &ref, cur)) {
		const stile_t *stile
			= qmap_get(stile_hd, &ref);

		tm_render(stile->tm_ref,
				(p[0] - cam.x) * inc,
				(p[1] - cam.y) * inc,
				stile->tm_x,
				stile->tm_y,
				16.0 * cam.zoom,
				16.0 * cam.zoom);
	}
}

static inline void
mymap_put(int16_t x, int16_t y, int16_t z, unsigned ref)
{
	int16_t p[] = { x, y, z };
	geo_put(map_hd, p, ref, dim);
}

static inline unsigned
stile_add(unsigned tm_ref, uint16_t x, uint16_t y) {
	stile_t stile;

	stile.tm_ref = tm_ref;
	stile.tm_x = x;
	stile.tm_y = y;

	return qmap_put(stile_hd, NULL, &stile);
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
		int sprite = 1;
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

		stile_add(tm_ref, tm_x, tm_y);
	}

	uint16_t w, h, d;

	ret = fgets(line, sizeof(line), fp);
	word = line;

	CBUG(!ret || *line == '\n', "file input end: H\n");

	space = strchr(word, ' ');
	CBUG(!space, "file input end: I\n");
	*space = '\0';

	w = strtold(word, NULL);

	word = space + 1;
	space = strchr(word, ' ');
	CBUG(!space, "file input end: J\n");
	*space = '\0';

	h = strtold(word, NULL);

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
			space = strchr(line, '\n');
			CBUG(!space, "file input end: N\n");
			*space = '\0';
			fprintf(stderr, "line %s\n", line);

			for (uint16_t iw = 0; iw < w; iw ++, s++)
			{
				unsigned stile_ref = (unsigned) (*s - 'b');
				fprintf(stderr, "put %u %d %d %d\n", stile_ref, iw, ih, id);
				mymap_put(iw, ih, id, stile_ref);
			}
		}
	}
}

void
view_init(void)
{
	unsigned qm_stile = qmap_reg(sizeof(stile_t));

	cam.x = 0;
	cam.y = 0;
	cam.zoom = 8;

	hw = 0.5 * ((double) be_width) - 8.0 * cam.zoom;
	hh = 0.5 * ((double) be_height) - 8.0 * cam.zoom;

	stile_hd = qmap_open(QM_HNDL, qm_stile,
			0xFF, QM_AINDEX);

	geo_init();
	map_hd = geo_open("map", 0x1FFF);

	view_load("./map.txt");
}
