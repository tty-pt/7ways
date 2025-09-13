#include "../include/draw.h"
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <limits.h>

#define BYTE 8

typedef struct {
	uint32_t size;
	uint8_t *canvas;
	uint8_t channels;
	int frame_buffer_fd;
	uint32_t min_x, min_y,
		 max_y;
} screen_t;

screen_t screen;
uint32_t be_width, be_height;

static inline
screen_t screen_new(void) {
	struct fb_var_screeninfo vinfo;

	int fb_fd = open("/dev/fb0", O_RDWR);
	if (fb_fd == -1) {
		perror("Error: cannot open framebuffer device");
		exit(1);
	}

	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		perror("Error reading variable information");
		close(fb_fd);
		exit(2);
	}

	int w = vinfo.xres;
	int h = vinfo.yres;
	int color_channels = vinfo.bits_per_pixel / BYTE;
	long screen_size = w * h;

	printf("Framebuffer size: width=%d, height=%d, channels=%d\n", w, h,
			color_channels);

	// We avoid mmap for double buffering simulation
	uint8_t *canvas = (uint8_t *)malloc(sizeof(uint8_t) * screen_size * color_channels);
	if (!canvas) {
		perror("Failed to allocate canvas buffer");
		close(fb_fd);
		exit(3);
	}

	screen_t ans;
	be_width = w;
	be_height = h;
	ans.canvas = canvas;
	ans.size = screen_size;
	ans.channels = color_channels;
	ans.frame_buffer_fd = fb_fd;

	pread(fb_fd, ans.canvas,
			(size_t) screen_size * ans.channels,
			0);

	return ans;
}

void be_render(draw_lambda_t *lambda,
		uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, void *ctx)
{
	uint32_t screen_size = screen.size;
	uint32_t sw = be_width;
	uint32_t sh = be_height;
	uint8_t channels = screen.channels;
	size_t offset = y * sw + x;
	uint8_t *start = &screen.canvas[0]
		+ offset * channels;
	uint8_t *pos = start;

	if (x < screen.min_x)
		screen.min_x = x;

	if (y < screen.min_y)
		screen.min_y = y;

	if (y + h > screen.max_y)
		screen.max_y = y + h;

	for (
			uint32_t kce = (y + h) * sw,
			kc = offset, kcm = kc + w;

			kc < kce;

			kc ++, pos += channels)
	{
		if (kc > kcm) {
			kc += sw - w - 1;
			if (kc >= kce)
				break;
			kcm = kc + w;
			pos = &screen.canvas[0]
				+ kc * channels;
		}

		uint32_t i = kc / sw;
		uint32_t j = kc % sw;
		uint32_t ix = j - x;
		uint32_t iy = sh - 1 - i;
		lambda(pos, ix, i - y, ctx);
	}
}

static inline
void screen_close(screen_t *screen) {
	free(screen->canvas);
	close(screen->frame_buffer_fd);
}

void be_init(void) {
	screen = screen_new();
}

void be_deinit(void) {
	screen_close(&screen);
}

void be_flush(void) {
	uint32_t sw = be_width;
	uint32_t h = screen.max_y - screen.min_y;
	size_t offset = screen.min_y * sw + screen.min_x;
	uint8_t *start = &screen.canvas[0]
		+ offset * screen.channels;

	lseek(screen.frame_buffer_fd,
			offset * screen.channels, SEEK_SET);

	write(screen.frame_buffer_fd,
			start, (h * sw) * screen.channels);
}
