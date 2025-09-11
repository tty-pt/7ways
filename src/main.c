#include "../include/draw.h"
#include <limits.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct {
	double time;
} ctx_t;

long long start_t;

static long long
timestamp(void)
{
	struct timeval te;
	gettimeofday(&te, NULL); // get current time
	return te.tv_sec * 1000000LL + te.tv_usec;
}

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

inline static double dt_get() {
	long long tick = timestamp();
	double ret = (tick - start_t) / 1000000.0;
	start_t = tick;
	return ret;
}

int main() {
	backend_t be = {
		.init = fb_init,
		.deinit = fb_deinit,
		.render = fb_render,
	};

	be.init(&be);
	double t = 0;

	start_t = timestamp();

	while (t < 3) {
		ctx_t ctx = { .time = t };

		be.render(&be, anime, 0, 0,
				be.width, be.height, &ctx);
		t += dt_get();
	}

	while (t < 5) {
		ctx_t ctx = { .time = t };

		be.render(&be, anime, 10, 10,
				100, 100, &ctx);
		t += dt_get();
	}

	uint32_t tw = be.width / 3, th = be.height / 3;

	while (t < 10) {
		ctx_t ctx = { .time = t };

		be.render(&be, anime, tw, th,
				tw, th, &ctx);
		t += dt_get();
	}

	be.deinit(&be);
	return 0;
}
