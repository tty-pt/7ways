#ifndef CHAR_H
#define CHAR_H

#include <stdint.h>

#define CHAR_SYNC 0

enum dir {
	DIR_DOWN,
	DIR_UP,
	DIR_LEFT,
	DIR_RIGHT,
};

enum anim {
	AN_WALK,
	AN_IDLE,
};

void char_init(void);
unsigned char_load(unsigned tm_ref, double x, double y);
enum dir char_dir(unsigned ref);
void char_face(unsigned ref, enum dir dir);
void char_animate(unsigned ref, enum anim anim);
enum anim char_animation(unsigned ref);
void char_render(unsigned ref);
void char_pos(double *x, double *y, unsigned ref);
void char_ipos(int16_t *p, unsigned ref);
int char_update(unsigned ref, double dt);
unsigned char_dialog(unsigned ref, char *text);
void char_talk(unsigned ref, enum dir dir);
void char_sync(void);

#endif
