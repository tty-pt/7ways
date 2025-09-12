#include "../include/img.h"

#include <qmap.h>
#include <qsys.h>
#include <string.h>

static unsigned img_be_hd;

void img_be_load(char *ext,
		img_load_t *load,
		img_render_t *render,
		img_free_t *free)
{
	img_be_t img_be = {
		.free = free,
		.load = load,
		.render = render
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
