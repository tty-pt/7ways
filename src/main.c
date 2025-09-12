#include "../include/draw.h"
#include "../include/img.h"
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

	color[2] = (uint8_t) (UCHAR_MAX * px);
	color[1] = (uint8_t) (UCHAR_MAX * py);
	color[0] = (uint8_t) 0;
	color[3] = UCHAR_MAX;
}

inline static double dt_get() {
	long long tick = timestamp();
	double ret = (tick - start_t) / 1000000.0;
	start_t = tick;
	return ret;
}

extern img_be_t png;
extern backend_t fb;

int main() {
	void *lamb = png.load("./resources/lamb.png");

	fb.init(&fb);
	double t = 0;

	start_t = timestamp();

	png.render(&fb, lamb, 20, 20, 20, 20, 200, 200);

	uint32_t tw = fb.width / 3, th = fb.height / 3;

	while (t < 10) {
		ctx_t ctx = { .time = t };

		fb.render(&fb, anime, tw, th,
				tw, th, &ctx);
		t += dt_get();
	}


	fb.deinit(&fb);
	png.free(lamb);
	return 0;
}
