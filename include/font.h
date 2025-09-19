#ifndef RPG_FONT_H
#define RPG_FONT_H

#include <stdint.h>

void font_init(void);
unsigned font_load(char *fname, uint32_t w, uint32_t h);

void
font_render_char(unsigned tm_ref,
		unsigned idx,
		uint32_t x, uint32_t y,
		uint32_t scale);

char *font_render(unsigned ref, char *text,
		uint32_t x, uint32_t y,
		uint32_t mx, uint32_t my,
		uint32_t scale);

#endif
