#include "../include/game.h"
#include "../include/shader.h"
#include "../include/img.h"
#include "../include/tile.h"
#include "../include/dialog.h"

#include <qsys.h>

unsigned cont = 1;
unsigned lamb;
int tile = -1;

extern double hw, hh;

int key_quit(unsigned short code,
		unsigned short type, int value)
{
	cont = 0;
	return 0;
}

int key_cont(unsigned short code,
		unsigned short type, int value)
{
	if (type)
		return 0;

	return vdialog_action();
}

static inline int
key_dialog_sel(unsigned short type, int down)
{
	if (type)
		return 0;

	return dialog_select(down);
}

int key_up(unsigned short code,
		unsigned short type, int value)
{
	return key_dialog_sel(type, 0);
}

int key_down(unsigned short code,
		unsigned short type, int value)
{
	return key_dialog_sel(type, 1);
}

unsigned dlg_tile;

int key_tile(unsigned short code,
		unsigned short type, int value)
{
	if (type || dialog_showing())
		return 0;

	dialog_start(dlg_tile);
	return 1;
}

void tile_sel(void) {
	char **args = dialog_args();
	tile = strtoull(args[0], NULL, 10);
}

/*
int key_info(unsigned short code,
		unsigned short type, int value)
{
	if (type || dialog_showing())
		return 0;

	dialog_start(dlg_info);
	return 1;
}
*/

void ensure_move(enum dir dir) {
	if (dialog_showing())
		return;

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

int input_default(unsigned short code,
		unsigned short value,
		int type)
{
	if (value)
		return 0;
	else
		return input_press(code);
}

int main() {
	double lamb_speed = 2;
	unsigned font;

	game_init();
	dialog_init();

	input_reg(KEY_Q, key_quit);

	input_reg(KEY_SPACE, key_cont);
	input_reg(KEY_ENTER, key_cont);
	input_reg(KEY_TAB, key_cont);

	input_reg(KEY_J, key_down);
	input_reg(KEY_DOWN, key_down);

	input_reg(KEY_K, key_up);
	input_reg(KEY_UP, key_up);

	input_reg(KEY_X, key_tile);

	view_load("./map.txt");

	unsigned font_img = img_load("./resources/font.png");
	dialog.font_tm = tm_load(font_img, 9, 21); 

	unsigned ui_img = img_load("./resources/Complete_UI_Essential_Pack_Free/01_Flat_Theme/Spritesheets/Spritesheet_UI_Flat.png");
	dialog.ui_box->ui_tm
		= tm_load(ui_img, 32, 32);

	lamb = 0;

	unsigned dlg = char_dialog(1, "Do you want to learn how to edit the map?");

	dialog_option(dlg, "No", "Oh. Ok, then. Smarty-pants.");
	unsigned dlg2 = dialog_option(dlg, "Yes", "Press 'x' to pick a tile to paint. Press 'i' to check your brush information, and 'z' to choose a layer.\n\nYou can place a tile in front of you using 'p'. If you want to paint the floor where you stand, use 'space'!\n\nDo you want to learn about editing dialog?");

	dialog_option(dlg2, "No", "Ok. Go ahead and paint!");
	dialog_option(dlg2, "Yes", "Press 't' to add a new dialog. Insert the text and when you are finished press 'tab'. The dialog ID will be shown.\nOptions are similar. You press 'o' to add a new one, and you will be asked to input a dialog ID to associate it to.\nPressing 'tab', you'll be asked to input the option's text. Doing it a third time, you'll be asked to insert the resulting dialog. Again, you'll get a dialog ID.\nGot all that?");

	dlg_tile = dialog_add("Select a tile number:");
	unsigned dlg_tile_sel
		= dialog_input(dlg_tile, 7, 1, IF_NUMERIC,
				"Tile '%1' selected");

	dialog_then(dlg_tile, tile_sel);

	game_start();

	while (cont) {
		shader_render();
		view_render();
		dialog_render();

		game_update();
		my_update();

		char_pos(&cam.x, &cam.y, lamb);
	}

	game_deinit();
	return 0;
}
