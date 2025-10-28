#include "../include/img.h"
#include "../include/gl.h"

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

	img_be_hd = qmap_open(NULL, NULL, QM_STR, qm_img_be, 0xF, 0);
	img_hd = qmap_open(NULL, NULL, QM_HNDL, qm_img, 0xF, QM_AINDEX);
	/* img_name_hd = qdb_open("img", QM_STR, QM_HNDL, 0xF, 0); */
	img_name_hd = qmap_open(NULL, NULL, QM_STR, QM_HNDL, 0xF, 0);

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

void draw_img_reg(uint32_t ref, uint8_t *data,
			 uint32_t w, uint32_t h);

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


	if (!(flags & IMG_LOAD))
		draw_img_reg(ref, img.data,
				img.w, img.h);

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

static inline uint8_t *
_img_pick(const img_t *img, uint32_t x, uint32_t y)
{
	uint8_t *pixel = &img->data[
		(y * img->w + x) * 4
	];

	return pixel;
}

void draw_img(uint32_t ref, int32_t x, int32_t y,
		      uint32_t dw, uint32_t dh)
{
	const img_t *img = qmap_get(img_hd, &ref);

	draw_img_ex(ref, x, y, 0, 0,
			img->w, img->h, dw, dh, default_tint);
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

	draw_img_upd(ref, x, y, 1, 1, color);
}

void
img_del(unsigned ref)
{
	gl_img_ureg(ref);

	const img_t *img = qmap_get(img_hd, &ref);
	qmap_del(img_name_hd, img->filename);
	free(img->filename);
	free(img->data);
	qmap_del(img_hd, &ref);
}
