#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

typedef struct {
	uint32_t size;
	uint8_t *canvas;
	uint8_t channels;
	uint32_t min_x, min_y, max_x, max_y;
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

void draw_img_ex(uint32_t ref, int32_t x, int32_t y,
		      uint32_t cx, uint32_t cy, uint32_t sw, uint32_t sh,
		      uint32_t dw, uint32_t dh, uint32_t tint);

void draw_img_upd(uint32_t ref, uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, uint8_t *data);

void draw_img(uint32_t ref, int32_t x, int32_t y,
		uint32_t dw, uint32_t dh);

#endif
