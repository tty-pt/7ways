#include "../include/game.h"
#include "../include/shader.h"
#include "../include/img.h"
#include "../include/tile.h"
#include "../include/dialog.h"
#include "../include/time.h"

#include <qsys.h>
#include <qdb.h>

unsigned start_cont = 1, cont = 1;
unsigned lamb;
unsigned tile, layer = 0;
unsigned dlg_edit, dlg_quit, dlg_save;
unsigned font_ref;

extern double hw, hh;

void quit_nosave(void) {
	cont = 0;
}

void quit_save(void) {
	view_sync();
	char_sync();
	cont = 0;
}

int key_quit(unsigned short code,
		unsigned short type, int value)
{
	dialog_start(dlg_quit);
	return 0;
}

int key_cont(unsigned short code,
		unsigned short type, int value)
{
	if (type) {
		if (start_cont) {
			start_cont = 0;
			return 1;
		}

		return 0;
	}

	if (vdialog_action())
		return 1;

	view_paint(lamb, tile, layer);
	return 0;
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

int dlg_key(unsigned short code,
		unsigned short type, unsigned dlg)
{
	if (type || dialog_showing())
		return 0;

	dialog_start(dlg);
	return 1;
}

int key_edit(unsigned short code,
		unsigned short type, int value)
{
	char **args = dialog_args();
	static char tile_s[BUFSIZ], layer_s[BUFSIZ];
	sprintf(tile_s, "%u", tile);
	sprintf(layer_s, "%u", layer);
	args[0] = tile_s;
	args[1] = layer_s;

	return dlg_key(code, type, dlg_edit);
}

void tile_sel(void) {
	char **args = dialog_args();
	tile = strtoull(args[0], NULL, 10);
}

void layer_sel(void) {
	char **args = dialog_args();
	tile = strtoull(args[0], NULL, 10);
}

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

	qdb_config.file = "./map.db";
	game_init();
	dialog_init();

	input_reg(KEY_ENTER, key_cont);
	input_reg(KEY_SPACE, key_cont);
	input_reg(KEY_TAB, key_cont);

	input_reg(KEY_Q, key_quit);

	input_reg(KEY_J, key_down);
	input_reg(KEY_DOWN, key_down);
	input_reg(KEY_K, key_up);
	input_reg(KEY_UP, key_up);

	input_reg(KEY_E, key_edit);

	view_load("./map.txt");

	view_layers[0] = 0;

	unsigned entry_ref = img_load("./resources/seven.png");
	unsigned press_ref = img_load("./resources/press.png");
	const img_t *entry_img = img_get(entry_ref);

	unsigned font_img = img_load("./resources/font.png");
	font_ref = tm_load(font_img, 9, 21); 

	unsigned ui_img = img_load("./resources/Complete_UI_Essential_Pack_Free/01_Flat_Theme/Spritesheets/Spritesheet_UI_Flat.png");
	dialog.ui_box->ui_tm
		= tm_load(ui_img, 32, 32);

	lamb = 0;

	dlg_quit = dialog_add("Really quit?");
	dialog_option(dlg_quit, "No", NULL);
	unsigned dlg_save_ask
		= dialog_option(dlg_quit, "Yes",
				"Save?");
	unsigned dlg_nosave
		= dialog_option(dlg_save_ask, "No",
				"See you later.");
	unsigned dlg_save
		= dialog_option(dlg_save_ask, "Yes",
				"Saving. Bye-bye.");

	dialog_then(dlg_nosave, quit_nosave);
	dialog_then(dlg_save, quit_save);

	dlg_edit = dialog_add("Select your edit action");

	unsigned dlg_tile
		= dialog_option(dlg_edit,
				"Tile",
				"Select tile number.");
	unsigned dlg_tile_sel
		= dialog_input(dlg_tile, 7, 1, IF_NUMERIC,
				"Tile '%1' selected");
	dialog_then(dlg_tile, tile_sel);

	unsigned dlg_layer
		= dialog_option(dlg_edit,
				"Layer",
				"Select layer number.");
	unsigned dlg_layer_set
		= dialog_input(dlg_layer, 7, 1, IF_NUMERIC,
				"Layer '%1' selected");
	dialog_then(dlg_layer, layer_sel);

	unsigned dlg_info
		= dialog_option(dlg_edit,
				"Info",
				"Tile: %1; Layer: %2");

	unsigned dlg = char_dialog(1, "Do you want to learn how to edit the map?");

	dialog_option(dlg, "No", "Oh. Ok, then. Smarty-pants.");
	unsigned dlg2 = dialog_option(dlg, "Yes", "Press 'e' to pick an edit action! You can place a tile in front of you using 'p'. If you want to paint the floor where you stand, use 'space'!\n\nDo you want to learn about editing dialog?");

	dialog_option(dlg2, "No", "Ok. Go ahead and paint!");
	dialog_option(dlg2, "Yes", "Press 't' to add a new dialog. Insert the text and when you are finished press 'tab'. The dialog ID will be shown.\nOptions are similar. You press 'o' to add a new one, and you will be asked to input a dialog ID to associate it to.\nPressing 'tab', you'll be asked to input the option's text. Doing it a third time, you'll be asked to insert the resulting dialog. Again, you'll get a dialog ID.\nGot all that?");

	game_start();

	while (start_cont) {
		img_render(entry_ref,
				0, 0,
				0, 0,
				entry_img->w,
				entry_img->h,
				be_width,
				be_height);


		uint8_t alpha = (time_tick * 200.0);

		img_tint(0x00FFFFFF
				| (((uint64_t) alpha) << 24));

		img_render(press_ref,
				be_width / 2 - 128,
				be_height - 128,
				0, 0,
				entry_img->w,
				entry_img->h,
				256,
				118);
		img_tint(default_tint);

		game_update();
		/* my_update(); */
	}

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
