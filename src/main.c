#include "../include/tile.h"
#include "../include/input.h"
#include "../include/view.h"
#include "../include/game.h"
#include "../include/shader.h"
#include "../include/char.h"

img_t lamb_img;
unsigned cont = 1;
unsigned lamb;

extern double hw, hh;

void key_quit(unsigned short code,
		unsigned short type, int value)
{
	cont = 0;
}

void ensure_move(enum dir dir) {
	if (char_animation(lamb) == AN_WALK)
		return;
	char_face(lamb, dir);
	char_animate(lamb, AN_WALK);
}

void
my_update(void) {
	if (key_value(KEY_J) || key_value(KEY_DOWN))
		ensure_move(DIR_DOWN);
	if (key_value(KEY_K) || key_value(KEY_UP))
		ensure_move(DIR_UP);
	if (key_value(KEY_H) || key_value(KEY_LEFT))
		ensure_move(DIR_LEFT);
	if (key_value(KEY_L) || key_value(KEY_RIGHT))
		ensure_move(DIR_RIGHT);
}

int main() {
	double lamb_speed = 2;

	game_init();

	input_reg(KEY_Q, key_quit);
	input_reg(KEY_ESC, key_quit);

	unsigned lamb_img = img_load("./resources/lamb.png");
	unsigned lamb_tm = tm_load(lamb_img, 32, 32);
	lamb = char_load(lamb_tm, 0, 0);

	game_start();

	while (cont) {
		shader_render();
		view_render();
		game_update();
		my_update();
		char_pos(&cam.x, &cam.y, lamb);
	}

	game_deinit();
	return 0;
}
