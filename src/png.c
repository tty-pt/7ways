#include "../include/img.h"
#include <limits.h>
#include <stdlib.h>
#include <qsys.h>
#include <png.h>

img_t
pngi_load(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	unsigned char header[8];
	png_structp png;
	img_t img;
	png_bytep *rows;
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

	 img.w = png_get_image_width(png, info);
	 img.h = png_get_image_height(png, info);
	 color_type = png_get_color_type(png, info);
	 bit_depth = png_get_bit_depth(png, info);

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

	 img.data = malloc(img.w * img.h * 4);

	 rows = malloc(sizeof(png_bytep) * img.h);

	 for (int y = 0; y < img.h; y++)
		 rows[y] = malloc(png_get_rowbytes(png,
					 info));

	 png_read_image(png, rows);

	 uint8_t *pos = img.data;

	 for (int y = 0; y < img.h; y++) {
		 png_bytep row = rows[y];

		 for (int x = 0; x < img.w; x++, pos += 4) {
			 png_byte *pixel = &(row[x * 4]);
			 pos[0] = pixel[0];
			 pos[1] = pixel[1];
			 pos[2] = pixel[2];
			 pos[3] = pixel[3];
		 }
	 }

	 fclose(fp);
	 png_destroy_read_struct(&png, &info, NULL);

	 for (int y = 0; y < img.h; y++)
		 free(rows[y]);

	 free(rows);

	 return img;
}
