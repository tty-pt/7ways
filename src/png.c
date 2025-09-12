#include "../include/img.h"
#include <limits.h>
#include <stdlib.h>
#include <qsys.h>
#include <png.h>

typedef struct {
	png_bytep *rows;
	int width;
	int height;
} pngi_t;

typedef struct {
	pngi_t *pngi;
	uint32_t cx, cy;
} pngi_ctx_t;

static void *
pngi_load(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	unsigned char header[8];
	png_structp png;
	pngi_t *img = malloc(sizeof(pngi_t));
	int color_type, bit_depth;

	CBUG(!fp, "fopen");

	fread(header, 1, 8, fp);
	CBUG(png_sig_cmp(header, 0, 8), "Not a PNG file");

	 png = png_create_read_struct(
			 PNG_LIBPNG_VER_STRING,
			 NULL, NULL, NULL);

	 CBUG(!png, "png_create_read_struct");

	 png_infop info = png_create_info_struct(png);
	 CBUG(!info, "png_create_info_struct");

	 CBUG(setjmp(png_jmpbuf(png)), "setjmp");

	 png_init_io(png, fp);
	 png_set_sig_bytes(png, 8);
	 png_read_info(png, info);

	 img->width      = png_get_image_width(png, info);
	 img->height     = png_get_image_height(png, info);
	 color_type = png_get_color_type(png, info);
	 bit_depth  = png_get_bit_depth(png, info);

	 if (bit_depth == 16)
		 png_set_strip_16(png);

	 if (color_type == PNG_COLOR_TYPE_PALETTE)
		 png_set_palette_to_rgb(png);

	 if (color_type == PNG_COLOR_TYPE_GRAY
			 && bit_depth < 8)
		 png_set_expand_gray_1_2_4_to_8(png);

	 if (png_get_valid(png, info, PNG_INFO_tRNS))
		 png_set_tRNS_to_alpha(png);

	 png_set_filler(png, 0xFF, PNG_FILLER_AFTER);


	 png_read_update_info(png, info);

	 img->rows = malloc(sizeof(png_bytep) * img->height);

	 for (int y = 0; y < img->height; y++)
		 img->rows[y] = malloc(png_get_rowbytes(png,
					 info));

	 png_read_image(png, img->rows);

	 fclose(fp);
	 png_destroy_read_struct(&png, &info, NULL);

	 return img;
}

static void
pngi_free(void *data)
{
	pngi_t *pngi = data;

	for (int y = 0; y < pngi->height; y++)
		free(pngi->rows[y]);

	free(pngi->rows);
	pngi->rows = NULL;
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
pngi_lambda(uint8_t *color,
		uint32_t x, uint32_t y,
		void *context)
{
	pngi_ctx_t *pngi_ctx = context;
	pngi_t *png = context;

	png_bytep row = pngi_ctx->pngi->rows[
		pngi_ctx->cy + y
	];

	png_byte *pixel = &(row[
			(pngi_ctx->cx + x) * 4
	]);

	color[2] = blend_u8(pixel[2], color[2], pixel[3]);
	color[1] = blend_u8(pixel[1], color[1], pixel[3]);
	color[0] = blend_u8(pixel[0], color[0], pixel[3]);
}

static void
pngi_render(void *png,
		uint32_t x, uint32_t y,
		uint32_t cx, uint32_t cy,
		uint32_t w, uint32_t h)
{
	pngi_ctx_t pngi_ctx = {
		.pngi = png,
		.cx = cx,
		.cy = cy,
	};

	be.render(pngi_lambda, x, y, w, h, &pngi_ctx);
}

void
png_init(void)
{
	img_be_load("png", pngi_load, pngi_render, pngi_free);
}
