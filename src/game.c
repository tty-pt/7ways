#include "../include/img.h"
#include "../include/gl.h"
#include "../include/input.h"
#include "../include/view.h"
#include "../include/time.h"
#include "../include/tile.h"
#include "../include/char.h"
#include "../include/font.h"

void png_init(void);
const uint8_t dim = 3;

void
game_init(void)
{
	img_init();
	png_init();
	img_load_all();
	be_init();
	view_init();
	tile_init();
	char_init();
	font_init();
	input_init(0);
}

void
game_deinit(void)
{
	input_deinit();
	gl_deinit();
	be_deinit();
	img_deinit();
}

double
game_update(void) {
	double dt;
	/* be_flush(); */
	gl_flush();
	be_flush();
	dt = dt_get();
	view_update(dt);
	input_poll();
	return dt_get();
}

void
game_start(void)
{
	time_init();
}
