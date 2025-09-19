#ifndef VIEW_H
#define VIEW_H

#include "cam.h"

extern double view_mul, view_hw, view_hh;
extern cam_t cam;

void view_render(void);
void view_init(void);
void view_load(char *fname);
void view_update(double dt);

unsigned vchar_load(unsigned tm_ref, double x, double y);

#endif
