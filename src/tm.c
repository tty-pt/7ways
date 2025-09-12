#include "../include/tilemap.h"

tm_t
tm_load(char *filename, uint32_t w, uint32_t h)
{
	tm_t tm;

	tm.img = img_load(filename);
	tm.w = w;
	tm.h = h;
	tm.nx = tm.img.w / w;
	tm.ny = tm.img.h / h;

	return tm;
}
