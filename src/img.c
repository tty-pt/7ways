#include "../include/img.h"

#include <qmap.h>
#include <qsys.h>
#include <string.h>

typedef struct {
	const img_t *img;
	uint32_t cx, cy, sw, sh, dw, dh,
		 doffx, doffy;
	uint32_t tint;
} img_ctx_t;

static unsigned img_be_hd, img_hd;
static uint32_t tint;

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
	unsigned qm_img_be = qmap_reg(sizeof(img_be_t)),
		 qm_img = qmap_reg(sizeof(img_t));

	img_be_hd = qmap_open(QM_STR, qm_img_be, 0xF, 0);
	img_hd = qmap_open(QM_HNDL, qm_img, 0xF, QM_AINDEX);
	tint = default_tint;
}

void
img_deinit(void)
{
	unsigned cur = qmap_iter(img_hd, NULL, 0);
	const void *key, *value;

	while (qmap_next(&key, &value, cur))
		img_free((img_t *) value);
}

unsigned img_load(char *filename) {
	char *ext = strrchr(filename, '.');
	img_be_t *be;
	img_t ret;

	CBUG(!ext, "IMG: invalid filename %s\n", filename);

	be = (img_be_t *) qmap_get(img_be_hd, ext + 1);
	CBUG(!be, "IMG: %s backend not present.\n", ext);

	ret = be->load(filename);
	ret.be = be;

	unsigned ref = qmap_put(img_hd, NULL, &ret);

	WARN("img_load %u: %s\n", ref, filename);
	return ref;
}

const img_t *
img_get(unsigned ref)
{
	return qmap_get(img_hd, &ref);
}

static inline uint8_t mul255(uint8_t a, uint8_t b)
{
    return (uint8_t)((a * (int)b + 127) / 255);
}

static inline uint8_t
blend_u8(uint8_t s, uint8_t d, uint8_t a)
{
    return (uint8_t) ((s * (int) a
			    + d * (int)(255 - a)
			    + 127) / 255
		    );
}

static inline uint32_t
map_coord_off(uint32_t d, uint32_t doff, uint32_t D_full,
		uint32_t S)
{
	if (D_full <= 1 || S == 0)
		return 0;

	return (uint32_t)(((uint64_t) (d + doff) * (S - 1))
			/ (D_full - 1));
}

static void
img_lambda(uint8_t *color,
		uint32_t x, uint32_t y,
		void *context)
{
	img_ctx_t *c = context;
	uint32_t sx = c->cx + map_coord_off(x, c->doffx,
			c->dw, c->sw);
	uint32_t sy = c->cy + map_coord_off(y, c->doffy,
			c->dh, c->sh);

	/* clamp defensivo */
	if (sx >= c->img->w)
		sx = c->img->w - 1;

	if (sy >= c->img->h)
		sy = c->img->h - 1;

	uint8_t *pixel = &c->img->data[
		(sy * c->img->w + sx) * 4
	];

	/* color[0] = blend_u8(pixel[2], color[0], pixel[3]); */
	/* color[1] = blend_u8(pixel[1], color[1], pixel[3]); */
	/* color[2] = blend_u8(pixel[0], color[2], pixel[3]); */

	uint8_t ta = (uint8_t)((c->tint >> 24) & 0xFF);
	uint8_t tr = (uint8_t)((c->tint >> 16) & 0xFF);
	uint8_t tg = (uint8_t)((c->tint >>  8) & 0xFF);
	uint8_t tb = (uint8_t)((c->tint >>  0) & 0xFF);

	uint8_t a  = mul255(pixel[3], ta);
	uint8_t sr = mul255(pixel[0], tr);
	uint8_t sg = mul255(pixel[1], tg);
	uint8_t sb = mul255(pixel[2], tb);

	color[0] = blend_u8(sb, color[0], a);
	color[1] = blend_u8(sg, color[1], a);
	color[2] = blend_u8(sr, color[2], a);
}

void
img_render(unsigned img_ref,
                 int32_t x, int32_t y,
                 uint32_t cx, uint32_t cy,
                 uint32_t sw, uint32_t sh,
                 uint32_t dw, uint32_t dh)
{
	const img_t *img = img_get(img_ref);
	uint32_t full_dw = dw, full_dh = dh;
	uint32_t doffx = 0, doffy = 0;

	if (x < 0) {
		uint32_t cut = (uint32_t)(-x);

		if (cut >= dw)
			return;

		doffx = cut;
		x += (int32_t) cut;
		dw -= cut;
	}

	if (y < 0) {
		uint32_t cut = (uint32_t)(-y);

		if (cut >= dh)
			return;

		doffy = cut;
		y += (int32_t)cut;
		dh -= cut;
	}

	img_ctx_t ctx = {
		.img = img,
		.cx = cx, .cy = cy,
		.sw = sw, .sh = sh,
		.dw = full_dw, .dh = full_dh,
		.doffx = doffx, .doffy = doffy,
		.tint = tint,
	};

	be_render(img_lambda, x, y, dw, dh, &ctx);
}

void
img_tint(uint32_t atint)
{
	tint = atint;
}
