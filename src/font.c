#include "../include/font.h"
#include "../include/tile.h"
#include "../include/cam.h"

#include <string.h>

#include <ttypt/qmap.h>
#include <ttypt/qsys.h>

typedef struct {
	unsigned tm_ref;
} font_t;

unsigned font_hd;
extern cam_t cam;

void
font_init(void)
{
	/* unsigned qm_font = qmap_reg(sizeof(font_t)); */
	/* font_hd = qmap_open(QM_HNDL, qm_font, 0xF, QM_AINDEX); */
}

void
font_render_char(unsigned tm_ref,
		unsigned idx,
		uint32_t x, uint32_t y,
		uint32_t scale)
{
	const tm_t *tm = tm_get(tm_ref);
	unsigned nx = idx % tm->nx,
		 ny = idx / tm->nx;

	tm_render(tm_ref,
			x, y,
			nx, ny,
			tm->w * scale, tm->h * scale,
			1, 1);
}


char *
font_render(unsigned tm_ref, char *text,
		uint32_t x, uint32_t y,
		uint32_t mx, uint32_t my,
		uint32_t scale)
{
	const tm_t *tm = tm_get(tm_ref);
	uint32_t ix = x;

	for (char *s = text; *s; s++, ix += tm->w * scale) {
		uint32_t ni, nx, ny;
		char *space = strchr(s, ' ');

		if (!space) {
			space = strchr(s, '-');
			if (space)
				space++;
			else
				space = strchr(s, '\n');
		}

		if (*s == '\n' || ix + (space - s) * tm->w * scale >= mx) {
			y += tm->h * scale;
			if (y + tm->h * scale >= my)
				return s + 1;
			ix = x;
		}
		
		if (*s == ' ')
			continue;

		if (*s <= 127)
			ni = *s - '!';
		else
			ni = 96 + (*s - (char) 192);

		ny = ni / tm->nx;
		nx = ni % tm->nx;

		tm_render(tm_ref,
				ix, y,
				nx, ny,
				tm->w * scale, tm->h * scale,
				1, 1);
	}

	return NULL;
}
