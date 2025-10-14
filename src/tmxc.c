#include <stdio.h>
#include <tmx.h>

#include "../include/map.h"
#include "../include/img.h"

tmx_map *map;

void png_init(void);
void be_render(draw_lambda_t *lambda,
		int32_t x, int32_t y,
		uint32_t w, uint32_t h, void *ctx)
{
}

int
main(int argc, char *argv[])
{
	uint8_t *data;
	unsigned ts_n, layer_n, above_n;
	char filename[BUFSIZ];
	FILE *fp = fopen("./map/info.txt", "w");

	map = tmx_load("./map/map.tmx");
	img_init();
	png_init();

	layer_n = above_n = 0;
	for (tmx_layer *layer = map->ly_head;
			layer;
			layer = layer->next
	    ) {
		if (layer->type != L_LAYER)
			continue;

		tmx_property *prop
			= tmx_get_property(layer->properties,
					"above");

		snprintf(filename, sizeof(filename),
				"./map/map%u.png",
				layer_n + above_n);

		unsigned bm_ref = img_new(&data, filename,
				map->width, map->height, 0);

		if (prop)
			above_n++;
		else
			layer_n++;


		for (uint32_t y = 0; y < map->height; y++)
			for (uint32_t x = 0; x < map->width; x++)
			{
				unsigned gid = (layer->content.gids[
						(y * map->width)
						+ x
				]) & TMX_FLIP_BITS_REMOVAL;

				if (map->tiles[gid] == NULL) {
					img_paint(bm_ref, x, y, 0);
					continue;
				}

				unsigned idx = gid
					- map->ts_head->firstgid;

				uint32_t color = map_color(idx);

				img_paint(bm_ref, x, y, color);
			}

		img_save(bm_ref);
	}

	fprintf(fp, "%u %u %u %u",
			map->width,
			map->height,
			layer_n, above_n);

	ts_n = 0;
	for (tmx_tileset_list *tsl = map->ts_head;
			tsl;
			tsl = tsl->next, ts_n++)
	{
		tmx_tileset *ts = tsl->tileset;
		tmx_tile *tile;
		uint32_t w = ts->image->width / ts->tile_width,
			 h = ts->image->height / ts->tile_height;

		snprintf(filename, sizeof(filename),
				"./map/col%u.png",
				ts_n);

		unsigned bm_ref = img_new(&data, filename, w, h, 0);
		tile = ts->tiles;
		for (uint32_t y = 0; y < h; y++)
			for (uint32_t x = 0; x < w; x++, tile++) {
				if (!tile->properties) {
					img_paint(bm_ref, x, y, 0xFF000000);
					continue;
				}

				tmx_property *prop
					= tmx_get_property(tile->properties, "col");

				img_paint(bm_ref, x, y, prop
						? 0xFFFFFFFF
						: 0xFF000000);
			}

		img_save(bm_ref);
	}

	fprintf(fp, " %u\n",
			ts_n);

	for (tmx_tileset_list *tsl = map->ts_head;
			tsl;
			tsl = tsl->next)
	{
		tmx_tileset *ts = tsl->tileset;

		fprintf(fp, "%u %u %s\n",
				ts->tile_width,
				ts->tile_height,
				ts->image->source);
	}

	fclose(fp);
	return 0;
}
