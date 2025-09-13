#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

struct backend;

typedef void draw_lambda_t(uint8_t *color,
		uint32_t x, uint32_t y,
		void *ctx);

typedef void draw_render_t(draw_lambda_t *lambda,
		uint32_t x, uint32_t y,
		uint32_t w, uint32_t h,
		void *ctx);

typedef void draw_init_t(void);

typedef struct backend {
	draw_init_t *init, *deinit, *flush;
	draw_render_t *render;
	uint32_t width, height;
} backend_t;

extern backend_t be;

#endif
