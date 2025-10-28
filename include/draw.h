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

/**
 * @brief (Optional) Informs the backend to
 * create a hardware texture for the given
 * image reference.
 */
void be_register_texture(
		unsigned ref, uint8_t *data,
		uint32_t w, uint32_t h);

/**
 * @brief (Optional) Informs the backend to
 * delete a hardware texture.
 */
void be_unregister_texture(unsigned ref);

/**
 * @brief (Optional) Informs the backend that
 * a portion of a texture's CPU-side data has
 * changed and needs re-uploading.
 * Used by img_paint.
 */
void be_update_texture_rect(unsigned ref,
		uint32_t x, uint32_t y,
		uint32_t w, uint32_t h,
		uint8_t *data_rect_start);

void __attribute__((weak))
be_render_img_ex(unsigned ref,
		int32_t x, int32_t y,
		uint32_t cx, uint32_t cy,
		uint32_t sw, uint32_t sh,
		uint32_t dw, uint32_t dh,
		uint32_t tint);

#endif
