#ifndef RPG_IMG_H
#define RPG_IMG_H

#include "./draw.h"
#include <stdlib.h>

enum img_new_flags {
	IMG_LOAD,
};

typedef unsigned img_load_t(const char *filename);
typedef int img_save_t(const char *filename,
		const uint8_t *data, uint32_t w, uint32_t h);

typedef struct img_be {
	img_load_t *load;
	img_save_t *save;
} img_be_t;

void img_be_load(char *ext, img_load_t *load, img_save_t *save);

static const uint32_t default_tint = 0xFFFFFFFF;

void img_init(void);
void img_load_all(void);
void img_deinit(void);
unsigned img_load(const char *filename);
void img_save(unsigned ref);
void img_size(uint32_t *w, uint32_t *h, unsigned ref);

void img_render(unsigned ref,
		int32_t x, int32_t y,
		uint32_t dw, uint32_t dh);

void img_render_ex(unsigned ref,
		int32_t x, int32_t y,
		uint32_t cx, uint32_t cy,
		uint32_t sw, uint32_t sh,
		uint32_t dw, uint32_t dh);

void img_tint(uint32_t tint);

// image painting
unsigned img_new(uint8_t **data,
		const char *filename,
		uint32_t w, uint32_t h,
		unsigned flags);
uint32_t img_pick(unsigned ref, uint32_t x, uint32_t y);
void img_paint(unsigned ref,
		uint32_t x, uint32_t y,
		uint32_t color);

void img_del(unsigned ref);

#endif
