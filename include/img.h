#ifndef RPG_IMG_H
#define RPG_IMG_H

#include "./draw.h"

struct img_be;

typedef void *img_load_t(const char *filename);
typedef void img_free_t(void *data);

typedef struct img_be {
	draw_lambda_t *lambda;
	img_load_t *load;
	img_free_t *free;
} img_be_t;

#endif
