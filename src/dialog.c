#include "../include/dialog.h"
#include "../include/tile.h"
#include "../include/time.h"
#include "../include/input.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <ttypt/qmap.h>
#include <ttypt/idm.h>
#include <ttypt/qsys.h>

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

typedef struct {
	char *text, *next;
	ids_t  options;
	uint8_t option, option_n;
	unsigned input;
	dialog_cb_t *then;
} dialog_t;

typedef struct {
	char *text;
	unsigned ref;
} option_t;

typedef struct {
	char text[BUFSIZ];
	uint32_t cw, ch;
	unsigned next, len, flags;
} input_t;

unsigned dialog_hd, option_hd, input_hd;

ui_box_t dialog_box = {
	.scale = 3,
	.idx = {
		23 + 1, 23 + 2, 23 + 3,
		46 + 1, 46 + 2, 46 + 3,
		69 + 1, 69 + 2, 69 + 3,
		92 + 1, 92 + 2, 92 + 3,
		115 + 1,
	},
	.pad = 0.2,
};

dialog_t cdialog;
char *dialog_arg[8];
unsigned dialog_arg_n;
ids_t dialog_seq;

dialog_settings_t dialog = {
	.ui_box = &dialog_box,
	.font_scale = 2,
};

char key_chars[] = {
	[GLFW_KEY_1] = '1',
	[GLFW_KEY_2] = '2',
	[GLFW_KEY_3] = '3',
	[GLFW_KEY_4] = '4',
	[GLFW_KEY_5] = '5',
	[GLFW_KEY_6] = '6',
	[GLFW_KEY_7] = '7',
	[GLFW_KEY_8] = '8',
	[GLFW_KEY_9] = '9',
	[GLFW_KEY_0] = '0',
	[GLFW_KEY_A] = 'a',
	[GLFW_KEY_B] = 'b',
	[GLFW_KEY_C] = 'c',
	[GLFW_KEY_D] = 'd',
	[GLFW_KEY_E] = 'e',
	[GLFW_KEY_F] = 'f',
	[GLFW_KEY_G] = 'g',
	[GLFW_KEY_H] = 'h',
	[GLFW_KEY_I] = 'i',
	[GLFW_KEY_J] = 'j',
	[GLFW_KEY_K] = 'k',
	[GLFW_KEY_L] = 'l',
	[GLFW_KEY_M] = 'm',
	[GLFW_KEY_N] = 'n',
	[GLFW_KEY_O] = 'o',
	[GLFW_KEY_P] = 'p',
	[GLFW_KEY_Q] = 'q',
	[GLFW_KEY_R] = 't',
	[GLFW_KEY_S] = 's',
	[GLFW_KEY_T] = 't',
	[GLFW_KEY_U] = 'u',
	[GLFW_KEY_V] = 'v',
	[GLFW_KEY_W] = 'w',
	[GLFW_KEY_X] = 'x',
	[GLFW_KEY_Y] = 'y',
	[GLFW_KEY_Z] = 'z',
	[GLFW_KEY_SPACE] = ' ',
	[GLFW_KEY_BACKSPACE] = '\b',
	[GLFW_KEY_ENTER] = '\n',
};
static inline void
box_row(unsigned start_idx, ui_box_t *ui_box,
		unsigned ix, unsigned iy,
		unsigned w, unsigned h,
		unsigned nx, unsigned mx)
{
	unsigned idx = ui_box->idx[start_idx];

	tm_render(ui_box->ui_tm, ix, iy,
			idx % nx,
			idx / nx,
			w, h, 1, 1);
	ix += w;

	while (ix + w < mx) {
		idx = ui_box->idx[1 + start_idx];

		tm_render(ui_box->ui_tm, ix, iy,
				idx % nx,
				idx / nx,
				w, h, 1, 1);
		ix += w;
	};

	idx = ui_box->idx[2 + start_idx];

	tm_render(ui_box->ui_tm, ix, iy,
			idx % nx,
			idx / nx,
			w, h, 1, 1);

	iy += h;
}

void
box_render(ui_box_t *ui_box,
		uint32_t x, uint32_t y,
		uint32_t mx, uint32_t my)
{
	const tm_t *tm = tm_get(ui_box->ui_tm);
	uint32_t idx;
	uint32_t h = ui_box->scale * tm->h,
		 w = ui_box->scale * tm->w,
		 iy = y;

	if (iy + h >= my)
		goto small_height;

	box_row(0, ui_box, x, y, w, h, tm->nx, mx);
	iy += h;

	for (; iy + h < my; iy += h)
		box_row(3, ui_box, x, iy,
				w, h, tm->nx, mx);

	box_row(6, ui_box, x, iy,
			w, h, tm->nx, mx);

	return;

small_height:

	if (mx > x + w) {
		box_row(9, ui_box, x, iy,
				w, h, tm->nx, mx);
		return;
	}

	idx = ui_box->idx[12];
	tm_render(ui_box->ui_tm, x, y,
			idx % tm->nx,
			idx / tm->nx,
			w, h, 1, 1);
}

static inline void
box_d(
		long *dx_r, long *dy_r,
		ui_box_t *box,
		unsigned lx,
		unsigned ly,
		unsigned add)
{
	const tm_t *tm = tm_get(dialog.ui_box->ui_tm);
	uint32_t h = box->scale * tm->h,
		 w = box->scale * tm->w;

	long dx = lx;
	long dy = ly;
	unsigned ax = add + dx / w;
	unsigned ay = add + dy / w;

	*dx_r = dx - ax * w;
	*dy_r = dy - ay * h;
}

void
dialog_init(void)
{
	unsigned qm_dialog = qmap_reg(sizeof(dialog_t));
	unsigned qm_opt = qmap_reg(sizeof(option_t));
	unsigned qm_input = qmap_reg(sizeof(input_t));

	dialog_hd = qmap_open(QM_HNDL, qm_dialog, 0xFF, QM_AINDEX);
	option_hd = qmap_open(QM_HNDL, qm_opt, 0xFF, QM_AINDEX);
	input_hd = qmap_open(QM_HNDL, qm_input, 0xFF, QM_AINDEX);

	dialog_seq = ids_init();
}

unsigned
dialog_add(char *text)
{
	dialog_t d;
	d.text = text;
	d.options = ids_init();
	d.option_n = 0;
	d.input = QM_MISS;
	d.then = NULL;
	return qmap_put(dialog_hd, NULL, &d);
}

static inline unsigned
input_add(uint32_t cw, uint32_t ch,
		unsigned next, unsigned flags)
{
	input_t d;
	memset(&d, 0, sizeof(d));
	d.cw = cw;
	d.ch = ch;
	d.next = next;
	d.len = 0;
	d.flags = flags;
	return qmap_put(input_hd, NULL, &d);
}

unsigned
dialog_input(unsigned d_ref,
		uint32_t cw, uint32_t ch, unsigned flags, char *next)
{
	dialog_t *dialog = (dialog_t *)
		qmap_get(dialog_hd, &d_ref);

	unsigned next_ref = dialog_add(next);
	unsigned input_ref = input_add(cw, ch, next_ref, flags);

	dialog->input = input_ref;

	return next_ref;
}

void
dialog_then(unsigned ref, dialog_cb_t *cb)
{
	dialog_t *dialog
		= (dialog_t *) qmap_get(dialog_hd, &ref);

	dialog->then = cb;
}

static unsigned
option_add(char *text, unsigned ref)
{
	option_t d;
	d.text = text;
	d.ref = ref;
	return qmap_put(option_hd, NULL, &d);
}

unsigned
dialog_option(unsigned ref, char *op_text, char *text)
{
	dialog_t *d = (dialog_t *)
		qmap_get(dialog_hd, &ref);

	unsigned new_ref = text ? dialog_add(text) : QM_MISS;
	unsigned new_o_ref = option_add(op_text, new_ref);

	ids_push(&d->options, new_o_ref);
	d->option_n++;
	return new_ref;
}

char *
dialog_snprintf(char *fmt) {
	static char res[BUFSIZ * 2];
	char *s, *r, *e;
	unsigned n;

	for (s = fmt, r = res; *s; r++, s++)
	{
		if (*s == '%' && isdigit(*(s + 1))) {
			n = strtoull(s + 1, &e, 10) - 1;
			strcpy(r, dialog_arg[n]);
			r += strlen(dialog_arg[n]) - 1;
			s = e - 1;
			continue;
		}

		*r = *s;
	}

	*r = '\0';
	return res;
}

char **dialog_args(void)
{
	return (char **) dialog_arg;
}

static inline void
_dialog_start(unsigned ref)
{
	const dialog_t *dialog = qmap_get(dialog_hd, &ref);

	if (cdialog.text && cdialog.then)
		cdialog.then();

	if (!dialog)
		return;

	ids_push(&dialog_seq, ref);
	memset(&cdialog, 0, sizeof(cdialog));
	cdialog = *dialog;
	cdialog.text = dialog_snprintf(dialog->text);
	cdialog.next = NULL;
	cdialog.option = 0;

	if (cdialog.input == QM_MISS)
		return;

	input_t *i = (input_t *)
		qmap_get(dialog_hd, &dialog->input);

	*i->text = '\0';
}

void
dialog_start(unsigned ref)
{
	if (cdialog.text)
		return;

	ids_drop(&dialog_seq);
	dialog_arg_n = 0;
	_dialog_start(ref);
}

static void
box_padding(uint32_t *px, uint32_t *py, ui_box_t *box)
{
	const tm_t *tm = tm_get(box->ui_tm);

	uint32_t h = box->scale * tm->h,
		 w = box->scale * tm->w;

	*px = (long) (w * box->pad);
	*py = (long) (h * box->pad);
}

static inline void
box_in(uint32_t *cx, uint32_t *cy,
		uint32_t tw, uint32_t th)
{
	uint32_t px, py, bx, by;
	long dx, dy;

	box_d(&dx, &dy, dialog.ui_box, tw, th, 1);

	bx = (be_width - tw) / 2;
	by = (be_height - th) / 2;

	box_padding(&px, &py, dialog.ui_box);
	dx = -dx;
	dy = -dy;

	box_render(dialog.ui_box,
			bx - px - dx,
			by - px - dy,
			bx - px + dx + tw,
			by - py + dy + th);

	*cx = bx - dx;
	*cy = by - dy;
}

void
dialog_options_render(void) {
	const tm_t *font_tm = tm_get(font_ref);
	uint32_t h = dialog.font_scale * font_tm->h,
		 w = dialog.font_scale * font_tm->w;

	unsigned max_len = 0, n = 0, ref;
	uint32_t cx, cy;

	idsi_t *idsi = ids_iter(&cdialog.options);

	if (!idsi)
		return;

	while (ids_next(&ref, &idsi)) {
		const option_t *opt =
			qmap_get(option_hd, &ref);;

		unsigned len = strlen(opt->text);

		if (len > max_len)
			max_len = len;
	}

	img_tint(default_tint);
	box_in(&cx, &cy, max_len * w, cdialog.option_n * h);

	n = 0;
	idsi = ids_iter(&cdialog.options);
	for (; ids_next(&ref, &idsi); n++) { 
		const option_t *opt =
			qmap_get(option_hd, &ref);;

		if (n == cdialog.option)
			img_tint(0xDD000000);
		else
			img_tint(0x77000000);

		font_render(
				font_ref,
				opt->text,
				cx,
				cy + n * h,
				cx + w,
				cy + (n + 1) * h,
				dialog.font_scale);
	}
	img_tint(default_tint);

}

void
input_render(void)
{
	if (cdialog.input == QM_MISS)
		return;

	const input_t *input
		= qmap_get(input_hd, &cdialog.input);

	const tm_t *font_tm = tm_get(font_ref);
	uint32_t h = dialog.font_scale * font_tm->h,
		 w = dialog.font_scale * font_tm->w;
	uint32_t cx, cy, cw = input->cw, ch = input->ch;

	cw *= w;
	ch *= h;

	img_tint(default_tint);
	box_in(&cx, &cy, cw, ch);

	img_tint(0xDD000000);

	font_render(
			font_ref,
			(char *) input->text,
			cx,
			cy,
			cx + cw,
			cy + ch,
			dialog.font_scale);

	img_tint(default_tint);
}

void
dialog_render(void)
{
	const tm_t *tm = tm_get(dialog.ui_box->ui_tm);
	unsigned x = 20;
	unsigned y = be_height - 250;
	unsigned mx = be_width - 20;
	unsigned my = be_height - 10;
	long dx, dy;

	if (!cdialog.text)
		return;

	box_d(&dx, &dy, dialog.ui_box, mx - x, my - y, 0);

	x += dx / 2;
	mx -= dx / 2;

	y += dy / 2;
	my -= dy / 2;

	img_tint(default_tint);
	box_render(dialog.ui_box,
			x, y,
			mx,
			my);

	uint32_t px, py;
	box_padding(&px, &py, dialog.ui_box);

	x += (long) px;
	mx -= (long) px;

	y += (long) py;
	my -= (long) py;

	img_tint(0xAA000000);

	cdialog.next = font_render(
			font_ref,
			cdialog.text,
			x, y,
			mx, my,
			dialog.font_scale);

	if (cdialog.next) {
		if ((unsigned) round(time_tick * 2) & 1)
			font_render_char(
					font_ref, 158,
					mx - tm->w,
					my - tm->h,
					dialog.font_scale);
	} else if (cdialog.option_n)
		dialog_options_render();
	else if (cdialog.input != QM_MISS)
		input_render();

	img_tint(default_tint);
}

int
dialog_action(void) {
	if (!cdialog.text)
		return 0;

	if (cdialog.input != QM_MISS) {
		const input_t *input
			= qmap_get(input_hd, &cdialog.input);

		dialog_arg[dialog_arg_n]
			= (char *) input->text;

		dialog_arg_n++;

		if (input->next != QM_MISS) {
			_dialog_start(input->next);
			return 1;
		}
	}

	if (cdialog.next || !cdialog.option_n) {
		if (!cdialog.next && cdialog.then)
			cdialog.then();

		cdialog.text = cdialog.next;
		return 1;
	}

	idsi_t *idsi = ids_iter(&cdialog.options);
	unsigned ref;

	for (uint8_t i = 0; (
			ids_next(&ref, &idsi)
			&& i != cdialog.option
			); i++);

	const option_t *opt =
		qmap_get(option_hd, &ref);

	cdialog.text = NULL;
	_dialog_start(opt->ref);
	return 0;
}

int
dialog_showing(void)
{
	return !!cdialog.text;
}

int
dialog_select(int down)
{
	if (!cdialog.option_n)
		return 0;

	if (down) {
		if (cdialog.option + 1 < cdialog.option_n)
			cdialog.option++;
	} else if (cdialog.option > 0)
		cdialog.option--;

	return 1;
}

int
input_press(unsigned short code)
{
	if (!cdialog.text || cdialog.input == QM_MISS)
		return 0;

	input_t *input = (input_t *)
		qmap_get(input_hd, &cdialog.input);

	char ch = key_chars[code];

	if (ch == '\b') {
		if (input->len) {
			input->len --;
			input->text[input->len] = '\0';
		}
		return 1;
	}

	if (input->flags & IF_NUMERIC) {
		if (!isdigit(ch))
			return 0;
	}

	input->text[input->len] = ch;
	input->len += 1;
	return 1;
}
