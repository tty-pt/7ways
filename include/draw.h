#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

typedef struct {
	uint32_t size;
	uint8_t *canvas;
	uint8_t channels;
	uint32_t min_x, min_y, max_y;
} screen_t;

typedef void draw_lambda_t(uint8_t *color,
		uint32_t x, uint32_t y,
		void *ctx);

extern uint32_t be_width, be_height;
extern screen_t screen;

void be_init(void);

void be_render(draw_lambda_t *lambda,
		int32_t x, int32_t y,
		uint32_t w, uint32_t h,
		void *ctx);

void be_flush(void);
void be_deinit(void);

#endif
