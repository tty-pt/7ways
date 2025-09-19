#include "../include/img.h"
#include "../include/input.h"
#include "../include/view.h"
#include "../include/time.h"
#include "../include/tile.h"

img_t pngi_load(const char *filename);
uint8_t dim = 3;

void
game_init(void)
{
	img_init();
	img_be_load("png", pngi_load);
	be_init();
	tile_init();
	view_init();
	input_init(0);
}

void
game_deinit(void)
{
	input_deinit();
	be_deinit();
	img_deinit();
}

double
game_update(void) {
	double dt;
	be_flush();
	dt = dt_get();
	view_update(dt);
	input_poll();
	be_flush();
	return dt_get();
}

void
game_start(void)
{
	time_init();
}
