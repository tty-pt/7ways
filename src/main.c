#include "../include/draw.h"
#include <limits.h>

typedef struct {
	double time;
} ctx_t;

static void anime(uint8_t *color,
		uint32_t x, uint32_t y,
		backend_t *be,
		void *context)
{
	double time = ((ctx_t *) context)->time;

	double px = (double) x * time / be->width;
	double py = (double) y * time / be->height;

	color[0] = (uint8_t) (UCHAR_MAX * px);
	color[1] = (uint8_t) (UCHAR_MAX * py);
	color[2] = (uint8_t) 0;
}

void fb_init(backend_t *be);
void fb_deinit(backend_t *be);
void fb_render(backend_t *be, draw_lambda_t *lambda,
		uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, void *ctx);

int main() {
	backend_t be = {
		.init = fb_init,
		.deinit = fb_deinit,
		.render = fb_render,
	};

	be.init(&be);
	double t = 0;

	while (t < 3) {
		ctx_t ctx = { .time = t };

		be.render(&be, anime, 0, 0,
				be.width, be.height, &ctx);
		t += 0.01;
	}

	while (t < 5) {
		ctx_t ctx = { .time = t };

		be.render(&be, anime, 10, 10,
				100, 100, &ctx);
		t += 0.01;
	}

	uint32_t tw = be.width / 3, th = be.height / 3;

	while (t < 10) {
		ctx_t ctx = { .time = t };

		be.render(&be, anime, tw, th,
				tw, th, &ctx);
		t += 0.01;
	}

	be.deinit(&be);
	return 0;
}
