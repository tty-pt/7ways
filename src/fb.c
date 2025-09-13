#include "../include/draw.h"
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <limits.h>

#define BYTE 8

int fb_fd;

static inline
screen_t screen_new(void) {
	struct fb_var_screeninfo vinfo;

	fb_fd = open("/dev/fb0", O_RDWR);
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

	pread(fb_fd, ans.canvas,
			(size_t) screen_size * ans.channels,
			0);

	return ans;
}

static inline
void screen_close(screen_t *screen) {
	free(screen->canvas);
	close(fb_fd);
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

	lseek(fb_fd, offset * screen.channels, SEEK_SET);
	write(fb_fd, start, (h * sw) * screen.channels);
}
