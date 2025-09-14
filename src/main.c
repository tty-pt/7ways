#include "../include/draw.h"
#include "../include/sprite.h"
#include "../include/input.h"
#include <limits.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct {
	double time;
} ctx_t;

long long start_t;
double t;
sprite_t lamb;

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

void turn_down(unsigned short type, int value)
{
	lamb.n = 0;
}

void turn_up(unsigned short type, int value)
{
	lamb.n = 1;
}

void turn_left(unsigned short type, int value)
{
	lamb.n = 2;
}

void turn_right(unsigned short type, int value)
{
	lamb.n = 3;
}

int main() {
	uint32_t tw, th;

	img_init();
	img_be_load("png", pngi_load);

	input_gen_init();
	input_init(0);
	input_reg(35, turn_left);
	input_reg(36, turn_down);
	input_reg(37, turn_up);
	input_reg(38, turn_right);

	lamb.tm = tm_load("./resources/lamb.png", 32, 32);
	lamb.n = 3;
	lamb.speed = 7.0;

	be_init();
	t = 0;

	start_t = timestamp();

	tw = be_width / 3;
	th = be_height / 3;

	while (t < 30) {
		ctx_t ctx = { .time = t };

		be_render(anime, tw, th,
				tw, th, &ctx);

		sprite_render(&lamb, tw, th, 256, 256);

		input_poll();

		be_flush();

		t += dt_get();
	}

	input_deinit();
	be_deinit();
	img_free(&lamb.tm.img);
	return 0;
}
