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
		void *context)
{
	double time = ((ctx_t *) context)->time;

	double px = (double) x * time / be.width;
	double py = (double) y * time / be.height;

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

extern void png_init(void);

int main() {
	img_t lamb;
	double t;
	uint32_t tw, th;

	img_init();
	png_init();

	lamb = img_load("./resources/lamb.png");

	be.init();
	t = 0;

	start_t = timestamp();

	img_render(&lamb, 20, 20, 20, 20, 200, 200);

	tw = be.width / 3;
	th = be.height / 3;

	while (t < 10) {
		ctx_t ctx = { .time = t };

		be.render(anime, tw, th,
				tw, th, &ctx);
		t += dt_get();
	}

	be.deinit();
	img_free(&lamb);
	return 0;
}
