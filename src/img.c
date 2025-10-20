#include "../include/img.h"

#include <ttypt/qmap.h>
#include <ttypt/qsys.h>
#include <string.h>

typedef struct {
	char *filename;
	struct img_be *be;
	uint8_t *data;
	uint32_t w, h;
} img_t;

typedef struct {
	const img_t *img;
	uint32_t cx, cy, sw, sh, dw, dh,
		 doffx, doffy;
	uint32_t tint;
} img_ctx_t;

static unsigned img_be_hd, img_hd, img_name_hd;
static uint32_t tint;

void img_be_load(char *ext,
		img_load_t *load,
		img_save_t *save)
{
	img_be_t img_be = {
		.load = load,
		.save = save,
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
	/* img_name_hd = qdb_open("img", QM_STR, QM_HNDL, 0xF, 0); */
	img_name_hd = qmap_open(QM_STR, QM_HNDL, 0xF, 0);

	tint = default_tint;
}

void
img_load_all(void)
{
	unsigned cur;
	const void *key, *value;

	cur = qmap_iter(img_name_hd, NULL, 0);
	while (qmap_next(&key, &value, cur))
		img_load((const char *) key);
}

static inline void
img_free(img_t *img)
{
	free(img->filename);
	free(img->data);
}

void
img_deinit(void)
{
	unsigned cur;
	const void *key, *value;

	/* qdb_sync(img_name_hd); */
	cur = qmap_iter(img_hd, NULL, 0);

	while (qmap_next(&key, &value, cur))
		img_free((img_t *) value);
}

unsigned
img_new(uint8_t **data,
		const char *filename,
		uint32_t w, uint32_t h,
		unsigned flags UNUSED)
{
	img_t img;
	unsigned ref;
	const unsigned *ref_r;
	char *ext = strrchr(filename, '.');

	img.w = w;
	img.h = h;
	img.data = malloc(img.w * img.h * 4);
	img.filename = strdup(filename);
	img.be = (img_be_t *) qmap_get(img_be_hd, ext + 1);

	if (data)
		*data = img.data;

	ref_r = qmap_get(img_name_hd, filename);
	ref = qmap_put(img_hd, ref_r, &img);
	qmap_put(img_name_hd, img.filename, &ref);

	return ref;
}

unsigned img_load(const char *filename) {
	char *ext = strrchr(filename, '.');
	img_be_t *be;
	unsigned ref;
	const unsigned *ref_r;
	img_t *img;

	ref_r = qmap_get(img_name_hd, filename);
	if (ref_r && qmap_get(img_hd, ref_r))
		return *ref_r;

	CBUG(!ext, "IMG: invalid filename %s\n", filename);

	be = (img_be_t *) qmap_get(img_be_hd, ext + 1);
	CBUG(!be, "IMG: %s backend not present.\n", ext);

	ref = be->load(filename);
	img = (img_t *) qmap_get(img_hd, &ref);
	img->be = be;

	WARN("img_load %u: %s\n", ref, filename);
	return ref;
}

void
img_save(unsigned ref)
{
	const img_t *img = qmap_get(img_hd, &ref);

	img->be->save(img->filename, img->data,
			img->w, img->h);
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

static inline uint8_t *
_img_pick(const img_t *img, uint32_t x, uint32_t y)
{
	uint8_t *pixel = &img->data[
		(y * img->w + x) * 4
	];

	return pixel;
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

	uint8_t *pixel = _img_pick(c->img, sx, sy);

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
img_render_ex(unsigned img_ref,
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
img_render(unsigned ref,
                 int32_t x, int32_t y,
                 uint32_t dw, uint32_t dh)
{
	const img_t *img = qmap_get(img_hd, &ref);

	img_render_ex(ref, x, y, 0, 0,
			img->w, img->h, dw, dh);
}

void
img_tint(uint32_t atint)
{
	tint = atint;
}

void
img_size(uint32_t *w, uint32_t *h, unsigned ref)
{
	const img_t *img = qmap_get(img_hd, &ref);

	*w = img->w;
	*h = img->h;
}

uint32_t
img_pick(unsigned ref, uint32_t x, uint32_t y)
{
	const img_t *img = qmap_get(img_hd, &ref);
	uint8_t *color = _img_pick(img, x, y);

	return color[0]
		| (color[1] << 8)
		| (color[2] << 16)
		| (color[3] << 24);
}

void
img_paint(unsigned ref, uint32_t x, uint32_t y, uint32_t c)
{
	const img_t *img = qmap_get(img_hd, &ref);
	uint8_t *color = _img_pick(img, x, y);

	color[0] = c & 0xFF;
	color[1] = (c >> 8) & 0xFF;
	color[2] = (c >> 16) & 0xFF;
	color[3] = (c >> 24) & 0xFF;
}

void
img_del(unsigned ref)
{
	const img_t *img = qmap_get(img_hd, &ref);
	qmap_del(img_name_hd, img->filename);
	free(img->filename);
	free(img->data);
	qmap_del(img_hd, &ref);
}
