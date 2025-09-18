#ifndef CHAR_H
#define CHAR_H

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

typedef struct {
	unsigned tm_ref;
	double x, y, nx, ny;
	uint8_t anim, dir;
} char_t;

unsigned char_load(unsigned tm_ref, double x, double y);
enum dir char_dir(unsigned ref);
void char_face(unsigned ref, enum dir dir);
void char_animate(unsigned ref, enum anim anim);
enum anim char_animation(unsigned ref);
void char_render(unsigned ref);
void char_pos(double *x, double *y, unsigned ref);

#endif
