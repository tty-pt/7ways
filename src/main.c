#include "../include/draw.h"
#include "../include/tm.h"
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

	double px = (double) x * time / be_width;
	double py = (double) y * time / be_height;

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

img_t pngi_load(const char *filename);

int main() {
	tm_t lamb;
	double t;
	uint32_t tw, th;

	img_init();
	img_be_load("png", pngi_load);

	lamb = tm_load("./resources/lamb.png", 32, 32);

	be_init();
	t = 0;

	start_t = timestamp();

	tw = be_width / 3;
	th = be_height / 3;

	while (t < 13) {
		ctx_t ctx = { .time = t };

		be_render(anime, 0, 0,
				tw, th, &ctx);

		tm_render(&lamb, tw, th,
				((int) (t * 7.0)) % 6, 3,
				256, 256);

		be_flush();

		t += dt_get();
	}

	be_deinit();
	img_free(&lamb.img);
	return 0;
}
