#ifndef RPG_IMG_H
#define RPG_IMG_H

#include "./draw.h"

struct img_be;

typedef void *img_load_t(const char *filename);
typedef void img_free_t(void *data);
typedef void img_render_t(backend_t *be, void *png,
		uint32_t x, uint32_t y,
		uint32_t cx, uint32_t cy,
		uint32_t w, uint32_t h);

typedef struct img_be {
	img_render_t *render;
	img_load_t *load;
	img_free_t *free;
} img_be_t;

#endif
