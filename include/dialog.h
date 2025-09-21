#ifndef DIALOG_H
#define DIALOG_H

#include "font.h"

typedef uint32_t box_idx_t[13];

typedef struct {
	unsigned ui_tm;
	box_idx_t idx;
	uint32_t scale;
	double pad;
} ui_box_t;

typedef struct {
	uint32_t font_scale;
	ui_box_t *ui_box;
} dialog_settings_t;

typedef void dialog_cb_t(void);

enum input_flags {
	IF_MULTILINE = 1,
	IF_NUMERIC = 2,
};

extern dialog_settings_t dialog;

void dialog_init(void);
unsigned dialog_add(char *text);
unsigned dialog_option(unsigned ref,
		char *op_text, char *text);
void dialog_start(unsigned ref);
void dialog_render(void);
int dialog_action(void);
int dialog_showing(void);
int dialog_select(int down);
void dialog_then(unsigned ref, dialog_cb_t *cb);
char **dialog_args(void);

unsigned dialog_input(unsigned d_ref,
		uint32_t cw, uint32_t ch,
		unsigned flags, char *next);

void input_start(unsigned ref);
int input_press(unsigned short code);

#endif
