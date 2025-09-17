#include "../include/draw.h"
#include "../include/tile.h"
#include "../include/cam.h"

#include <string.h>

#include <qmap.h>
#include <geo.h>

uint32_t hw, hh;
extern uint8_t dim;
unsigned map_hd;
cam_t cam;
img_t floors_img;
tm_t floors;
stile_t tile;
stile_t tiles[] = {{
	.tm_x = 12,
	.tm_y = 10,
}};

void
view_render(void)
{
	// how many tiles in the view
	uint8_t w = 1.0 + be_width / 16.0 / cam.zoom,
		h = 1.0 + be_height / 16.0 / cam.zoom;

	unsigned m = w * h * 2;
	unsigned mat[m];

	int16_t quad_s[dim], quad_e[dim];

	quad_s[0] = cam.x - 0.5 * w;
	quad_s[1] = cam.y - 0.5 * h;
	quad_s[2] = 0;

	quad_e[0] = quad_s[0] + w + 1;
	quad_e[1] = quad_s[1] + h + 1;
	quad_e[2] = 1;

	memset(mat, 0xFF, sizeof(mat));

	geo_search(map_hd, mat, quad_s, quad_e, dim);
	double sy = hh - cam.y * 16.0 * cam.zoom,
	       sx0 = hw - cam.x * 16.0 * cam.zoom;

	for (unsigned y = 0, i = 0; y < h; y++) {
		double sx = sx0;

		for (unsigned x = 0; x < w; x++, i++) {
			if (mat[i] == QM_MISS)
				continue;

			tm_render(&floors,
					sx,
					sy,
					tiles[0].tm_x,
					tiles[0].tm_y,
					16.0 * cam.zoom,
					16.0 * cam.zoom);

			sx += 1;
		}

		sy += 1;
	}
}

static inline void
mymap_put(int16_t x, int16_t y, int16_t z, unsigned ref)
{
	int16_t zero_p[] = { x, y, z };
	geo_put(map_hd, zero_p, 0, dim);
}

void
view_init(void)
{
	cam.x = 0;
	cam.y = 0;
	cam.zoom = 8;

	hw = be_width / 2.0 - 8.0 * cam.zoom;
	hh = be_height / 2.0 - 8.0 * cam.zoom;

	floors_img = img_load("./resources/Pixel Crawler - Free Pack/Environment/Tilesets/Floors_Tiles.png");
	floors = tm_load(&floors_img, 16, 16);
	tiles[0].tm = &floors;

	geo_init();
	map_hd = geo_open("map", 0x1FFF);

	mymap_put(0, 0, 0, 0);
}
