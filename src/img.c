#include "../include/img.h"

#include <qmap.h>
#include <qsys.h>
#include <string.h>

typedef struct {
	img_t *img;
	uint32_t cx, cy;
} img_ctx_t;

static unsigned img_be_hd;

void img_be_load(char *ext,
		img_load_t *load)
{
	img_be_t img_be = {
		.load = load,
	};

	qmap_put(img_be_hd, ext, &img_be);
}

void
img_init(void)
{
	unsigned qm_img_be = qmap_reg(sizeof(img_be_t));
	img_be_hd = qmap_open(QM_STR, qm_img_be, 0xF, 0);
}

img_t img_load(char *filename) {
	char *ext = strrchr(filename, '.');
	img_be_t *be;
	img_t ret;

	CBUG(!ext, "IMG: invalid filename %s\n", filename);

	be = (img_be_t *) qmap_get(img_be_hd, ext + 1);
	CBUG(!be, "IMG: %s backend not present.\n", ext);

	ret = be->load(filename);
	ret.be = be;
	return ret;
}

static inline uint8_t
blend_u8(uint8_t s, uint8_t d, uint8_t a)
{
    return (uint8_t) ((s * (int) a
			    + d * (int)(255 - a)
			    + 127) / 255
		    );
}

static void
img_lambda(uint8_t *color,
		uint32_t x, uint32_t y,
		void *context)
{
	img_ctx_t *pngi_ctx = context;

	uint8_t *pixel = &pngi_ctx->img->data[
		((pngi_ctx->cy + y) * pngi_ctx->img->w
		 + (pngi_ctx->cx + x)) * 4
	];

	color[2] = blend_u8(pixel[2], color[2], pixel[3]);
	color[1] = blend_u8(pixel[1], color[1], pixel[3]);
	color[0] = blend_u8(pixel[0], color[0], pixel[3]);
}

void
img_render(img_t *img,
		uint32_t x, uint32_t y,
		uint32_t cx, uint32_t cy,
		uint32_t w, uint32_t h)
{
	img_ctx_t pngi_ctx = {
		.img = img,
		.cx = cx,
		.cy = cy,
	};

	be.render(img_lambda, x, y, w, h, &pngi_ctx);
}
