#include "../include/shader.h"

#include "../include/draw.h"
#include "../include/time.h"

#include <stdint.h>
#include <limits.h>

typedef struct {
	double time;
} ctx_t;

extern uint32_t hw, hh;

static void shader_lambda(uint8_t *color,
		uint32_t x, uint32_t y,
		void *context)
{
	double time = ((ctx_t *) context)->time;

	double px = (double) x * time / be_width;
	double py = (double) y * time / be_height;

	color[2] = (uint8_t) (UCHAR_MAX * px);
	color[1] = (uint8_t) (UCHAR_MAX * py);
	color[0] = (uint8_t) 0;
	color[3] = UCHAR_MAX;
}

void shader_render(void) {
	ctx_t ctx = { .time = time_tick };

	be_render(shader_lambda, 0, 0,
			be_width, be_height, &ctx);
}
