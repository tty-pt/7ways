#include <ttypt/qgl.h>
#include <ttypt/qgl-font.h>
#include <ttypt/qgl-ui.h>
#include "../include/dialog.h"
#include "../include/time.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <ttypt/qmap.h>
#include <ttypt/idm.h>
#include <ttypt/qsys.h>

struct dialog {
	char *text;
	char *next;
	ids_t options;
	uint8_t option;
	uint8_t option_n;
	unsigned input;
	dialog_cb_t *then;
};

struct option {
	char *text;
	unsigned ref;
};

struct input {
	char text[BUFSIZ];
	uint32_t cw, ch;
	unsigned next, len, flags;
};

static unsigned dialog_hd, option_hd, input_hd;
static uint32_t be_width, be_height;

dialog_settings_t dialog = { .font_scale = 5 };
static qui_style_rule_t *g_css;

static qui_div_t *root, *txt;
static qui_style_t options_style, cursor_style;
static struct dialog cdialog;
static char *dialog_arg[8];
static unsigned dialog_arg_n;
static ids_t dialog_seq;
static int cursor_visible;

extern uint32_t font_ref;

static char *dialog_snprintf(char *fmt)
{
	static char res[BUFSIZ * 2];
	char *s, *r, *e;
	unsigned n;

	for (s = fmt, r = res; *s; r++, s++) {
		if (*s == '%' && isdigit((uint8_t)*(s + 1))) {
			n = (unsigned)strtoull(s + 1, &e, 10);
			if (n == 0 || n > dialog_arg_n) {
				*r = *s;
				continue;
			}
			const char *ins = dialog_arg[n - 1];
			size_t len = strlen(ins);
			memcpy(r, ins, len);
			r += len - 1;
			s = e - 1;
			continue;
		}
		*r = *s;
	}

	*r = '\0';
	return res;
}

static qui_div_t *build_qui_tree(void)
{
	root = qui_new(NULL, NULL);

	qui_div_t *overlay = qui_new(root, NULL);
	qui_class(overlay, "overlay");

	qui_div_t *panel = qui_new(root, NULL);
	qui_class(panel, "panel");

	txt = qui_new(panel, NULL);
	qui_class(txt, "text");
	qui_text(txt, cdialog.text);

	static char *arrow = "\x1F";
	qui_div_t *cursor = qui_new(panel, &cursor_style);
	if (((unsigned)round(time_tick * 2)) & 1)
		qui_text(cursor, arrow);
	else
		qui_text(cursor, " ");

	qui_div_t *options = qui_new(panel, &options_style);

	if (cdialog.option_n) {
		idsi_t *it = ids_iter(&cdialog.options);
		unsigned ref, idx = 0;

		for (; ids_next(&ref, &it); idx++) {
			const struct option *opt = qmap_get(option_hd, &ref);
			qui_div_t *row = qui_new(options, NULL);

			qui_class(row, (idx == cdialog.option) ?
				      "option-active" : "option");
			qui_text(row, opt->text);
		}
	}

	if (cdialog.input != QM_MISS) {
		const struct input *in = qmap_get(input_hd, &cdialog.input);
		qui_div_t *ibox = qui_new(panel, NULL);

		qui_class(ibox, "input");
		qui_text(ibox, in->text);
	}

	return root;
}

void qui_rebuild(void) {
	if (root)
		qui_clear(root);
	root = build_qui_tree();
	qui_apply_styles(root, g_css);
	const int margin = 10;

	qui_layout(root, margin, margin,
		   be_width - 2 * margin,
		   be_height - 2 * margin);

	cdialog.next = (char *)qui_overflow(txt);

	if (cdialog.next) {
		cursor_style.display = UI_DISPLAY_BLOCK;
		options_style.display = UI_DISPLAY_NONE;
	} else {
		cursor_style.display = UI_DISPLAY_NONE;
		if (cdialog.option_n)
			options_style.display = UI_DISPLAY_BLOCK;
	}
}

static void dialog_begin(unsigned ref)
{
	const struct dialog *dlg = qmap_get(dialog_hd, &ref);

	if (cdialog.text && cdialog.then)
		cdialog.then();
	if (!dlg)
		return;

	ids_push(&dialog_seq, ref);
	memset(&cdialog, 0, sizeof(cdialog));
	cdialog = *dlg;
	cdialog.text = dialog_snprintf(dlg->text);
	qui_rebuild();
}

static void css_reset(qui_style_t *s)
{
	memset(s, 0, sizeof(*s));
	s->font_scale = 5;
}

static void dialog_build_styles(void)
{
	qui_stylesheet_init(&g_css);

	qui_style_t s;

	css_reset(&s);
	s.bg_color = 0x66000000;
	qui_stylesheet_add(&g_css, "overlay", &s);

	css_reset(&options_style);
	options_style.position = UI_POS_ABSOLUTE;
	options_style.right = 35;
	options_style.bottom = 100;

	css_reset(&s);
	s.bg_color = 0xAA101010;
	s.border_color = 0xFFFFFFFF;
	s.border_size = 2;
	qui_stylesheet_add(&g_css, "panel", &s);

	css_reset(&s);
	s.font_ref = font_ref;
	s.font_scale = 2;
	s.text_color = 0xFFFFFFFF;
	s.pad_left = s.pad_right = s.pad_bottom = s.pad_top = 25;
	qui_stylesheet_add(&g_css, "text", &s);

	s.text_color = 0xFFEEEEEE;
	s.bg_color = 0;
	s.border_color = 0xFFFFFFFF;
	qui_stylesheet_add(&g_css, "option", &s);

	s.text_color = 0xFFFFFFFF;
	s.bg_color = 0x5500AAFF;
	qui_stylesheet_add(&g_css, "option-active", &s);

	s.bg_color = 0x55000000;
	s.position = UI_POS_ABSOLUTE;
	qui_stylesheet_add(&g_css, "input", &s);

	css_reset(&cursor_style);
	cursor_style.font_ref = font_ref;
	cursor_style.font_scale = 2;
	cursor_style.text_color = 0xFFFFFFFF;
	cursor_style.position = UI_POS_ABSOLUTE;
	cursor_style.display = UI_DISPLAY_NONE;
	cursor_style.right = 10;
	cursor_style.bottom = 5;
	qui_stylesheet_add(&g_css, "cursor", &cursor_style);
}

void dialog_init(void)
{
	unsigned qm_dialog = qmap_reg(sizeof(struct dialog));
	unsigned qm_opt = qmap_reg(sizeof(struct option));
	unsigned qm_input = qmap_reg(sizeof(struct input));

	dialog_hd = qmap_open(NULL, NULL, QM_HNDL, qm_dialog, 0xFF, QM_AINDEX);
	option_hd = qmap_open(NULL, NULL, QM_HNDL, qm_opt, 0xFF, QM_AINDEX);
	input_hd = qmap_open(NULL, NULL, QM_HNDL, qm_input, 0xFF, QM_AINDEX);

	dialog_seq = ids_init();
	qgl_size(&be_width, &be_height);
	dialog_build_styles();
}

unsigned dialog_add(char *text)
{
	struct dialog d;

	memset(&d, 0, sizeof(d));
	d.text = text;
	d.options = ids_init();
	d.input = QM_MISS;
	return qmap_put(dialog_hd, NULL, &d);
}

static inline unsigned input_add(uint32_t cw, uint32_t ch, unsigned next, unsigned flags)
{
	struct input d;

	memset(&d, 0, sizeof(d));
	d.cw = cw;
	d.ch = ch;
	d.next = next;
	d.flags = flags;
	return qmap_put(input_hd, NULL, &d);
}

unsigned dialog_input(unsigned d_ref, uint32_t cw, uint32_t ch,
		      unsigned flags, char *next)
{
	struct dialog *d = (struct dialog *)
		qmap_get(dialog_hd, &d_ref);

	unsigned next_ref = dialog_add(next);
	unsigned input_ref = input_add(cw, ch, next_ref, flags);

	d->input = input_ref;
	return next_ref;
}

void dialog_then(unsigned ref, dialog_cb_t *cb)
{
	struct dialog *d = (struct dialog *)
		qmap_get(dialog_hd, &ref);

	d->then = cb;
}

static unsigned option_add(char *text, unsigned ref)
{
	struct option d = { .text = text, .ref = ref };

	return qmap_put(option_hd, NULL, &d);
}

unsigned dialog_option(unsigned ref, char *op_text, char *text)
{
	struct dialog *d = (struct dialog *)
		qmap_get(dialog_hd, &ref);

	unsigned new_ref = text ? dialog_add(text) : QM_MISS;
	unsigned new_o_ref = option_add(op_text, new_ref);

	ids_push(&d->options, new_o_ref);
	d->option_n++;
	return new_ref;
}

void dialog_render(void)
{
	if (!cdialog.text)
		return;

	qui_render(root);
}

void dialog_start(unsigned ref)
{
	if (cdialog.text)
		return;

	ids_drop(&dialog_seq);
	dialog_arg_n = 0;
	dialog_begin(ref);
}

int dialog_action(void)
{
	if (!cdialog.text)
		return 0;

	if (cdialog.input != QM_MISS) {
		struct input *in = (struct input *)
			qmap_get(input_hd, &cdialog.input);

		dialog_arg[dialog_arg_n++] = in->text;
		if (in->next != QM_MISS) {
			dialog_begin(in->next);
			return 1;
		}
	}

	if (cdialog.next || !cdialog.option_n) {
		if (!cdialog.next && cdialog.then)
			options_style.display = UI_DISPLAY_NONE;
		cdialog.text = cdialog.next;
		qui_rebuild();
		return 1;
	}

	idsi_t *it = ids_iter(&cdialog.options);
	unsigned ref;
	for (uint8_t i = 0; ids_next(&ref, &it) && i != cdialog.option; i++)
		;
	const struct option *opt = qmap_get(option_hd, &ref);

	cdialog.text = NULL;
	dialog_begin(opt->ref);
	cursor_visible = 0;
	return 0;
}

int dialog_showing(void)
{
	return !!cdialog.text;
}

int dialog_select(int down)
{
	if (!cdialog.option_n)
		return 0;

	if (down) {
		if (cdialog.option + 1 < cdialog.option_n) {
			cdialog.option++;
			qui_rebuild();
		}
	} else if (cdialog.option > 0) {
		cdialog.option--;
		qui_rebuild();
	}

	return 1;
}

int input_press(unsigned short code)
{
	if (!cdialog.text || cdialog.input == QM_MISS)
		return 0;

	struct input *in = (struct input *)
		qmap_get(input_hd, &cdialog.input);

	in->len += qgl_key_parse(in->text, in->len, code, in->flags);
	qui_rebuild();
	return 1;
}

char **dialog_args(void)
{
	return (char **)dialog_arg;
}
