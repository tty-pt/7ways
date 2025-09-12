#ifndef RPG_IMG_H
#define RPG_IMG_H

#include "./draw.h"

struct img_be;

typedef void *img_load_t(const char *filename);
typedef void img_free_t(void *data);
typedef void img_render_t(void *png,
		uint32_t x, uint32_t y,
		uint32_t cx, uint32_t cy,
		uint32_t w, uint32_t h);

typedef struct img_be {
	img_render_t *render;
	img_load_t *load;
	img_free_t *free;
} img_be_t;

typedef struct {
	img_be_t *be;
	void *data;
} img_t;

void img_be_load(char *ext, img_load_t *load,
		img_render_t *render,
		img_free_t *free);

void img_init(void);
img_t img_load(char *filename);

static inline void
img_render(img_t *img,
		uint32_t x, uint32_t y,
		uint32_t cx, uint32_t cy,
		uint32_t w, uint32_t h)
{
	img->be->render(img->data, x, y, cx, cy, w, h);
}

static inline void
img_free(img_t *img)
{
	img->be->free(img->data);
}

#endif
