#ifndef VIEW_H
#define VIEW_H

#include "cam.h"
#include "char.h"

extern double view_mul, view_hw, view_hh;
extern cam_t cam;
extern unsigned view_layers[4];

void view_render(void);
void view_init(void);
void view_load(char *fname);
void view_update(double dt);
void view_paint(unsigned ref,
		unsigned tile, uint16_t layer);
int view_collides(double x, double y, enum dir dir);
int vdialog_action(void);
void view_sync(void);

unsigned vchar_load(unsigned tm_ref, double x, double y);

#endif
