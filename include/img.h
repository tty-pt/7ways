#ifndef RPG_IMG_H
#define RPG_IMG_H

#include "./draw.h"
#include <stdlib.h>

struct img_be;

typedef struct {
	struct img_be *be;
	uint8_t *data;
	uint32_t w, h;
} img_t;

typedef img_t img_load_t(const char *filename);

typedef struct img_be {
	img_load_t *load;
} img_be_t;

static const uint32_t default_tint = 0xFFFFFFFF;

void img_be_load(char *ext, img_load_t *load);

void img_init(void);
void img_deinit(void);
unsigned img_load(char *filename);
const img_t *img_get(unsigned ref);

void img_render(unsigned ref,
		int32_t x, int32_t y,
		uint32_t cx, uint32_t cy,
		uint32_t sw, uint32_t sh,
		uint32_t dw, uint32_t dh);

void img_tint(uint32_t tint);

static inline void
img_free(img_t *img)
{
	free(img->data);
}

#endif
