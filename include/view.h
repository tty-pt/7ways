#ifndef VIEW_H
#define VIEW_H

#include "cam.h"

extern cam_t cam;

void view_render(void);
void view_init(void);
void view_update(double dt);

#endif
