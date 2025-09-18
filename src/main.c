#include "../include/sprite.h"
#include "../include/input.h"
#include "../include/view.h"
#include "../include/game.h"
#include "../include/shader.h"

img_t lamb_img;
sprite_t lamb;

extern double hw, hh;

void turn_down(unsigned short code,
		unsigned short type, int value)
{
	lamb.n = 0;
}

void turn_up(unsigned short code,
		unsigned short type, int value)
{
	lamb.n = 1;
}

void turn_left(unsigned short code,
		unsigned short type, int value)
{
	lamb.n = 2;
}

void turn_right(unsigned short code,
		unsigned short type, int value)
{
	lamb.n = 3;
}

int main() {
	double dt;
	double lamb_speed = 2;

	game_init();

	input_reg(35, turn_left);
	input_reg(36, turn_down);
	input_reg(37, turn_up);
	input_reg(38, turn_right);

	unsigned lamb_img = img_load("./resources/lamb.png");
	lamb.tm_ref = tm_load(lamb_img, 32, 32);
	lamb.n = 3;
	lamb.speed = 7.0;

	game_start();

	while (time_tick < 30) {
		shader_render();
		view_render();

		sprite_render(&lamb,
				hw - 16.0 * cam.zoom,
				hh - 16.0 * cam.zoom,
				32 * cam.zoom,
				32 * cam.zoom);

		dt = game_events();

		switch (lamb.n) {
			case 0:
				cam.y += dt * lamb_speed;
				break;
			case 1:
				cam.y -= dt * lamb_speed;
				break;
			case 2:
				cam.x -= dt * lamb_speed;
				break;
			case 3:
				cam.x += dt * lamb_speed;
				break;
		}
	}

	game_deinit();
	return 0;
}
