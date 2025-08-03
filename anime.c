#include <fcntl.h>
#include <linux/fb.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BYTE 8
#define MAX_BYTE 255

typedef struct screen_struct {
  uint32_t size;
  uint32_t width;
  uint32_t height;
  uint8_t *canvas;
  uint8_t channels;
  int frame_buffer_fd;
} Screen;

Screen *new_screen() {
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
  long screen_size = w * h * color_channels;

  printf("Framebuffer size: width=%d, height=%d, channels=%d\n", w, h,
         color_channels);

  // We avoid mmap for double buffering simulation
  uint8_t *canvas = (uint8_t *)malloc(sizeof(uint8_t) * screen_size);
  if (!canvas) {
    perror("Failed to allocate canvas buffer");
    close(fb_fd);
    exit(3);
  }

  Screen *ans = malloc(sizeof(Screen));
  ans->width = w;
  ans->height = h;
  ans->canvas = canvas;
  ans->size = screen_size;
  ans->channels = color_channels;
  ans->frame_buffer_fd = fb_fd;

  return ans;
}

void close_screen(Screen *screen) {
  free(screen->canvas);
  close(screen->frame_buffer_fd);
  free(screen);
}

Screen *map(Screen *screen,
            uint8_t *(*lambda)(Screen *s, uint32_t x, uint32_t y, void *c),
            void *context) {
  uint32_t screen_size = screen->size;
  uint32_t w = screen->width;
  uint32_t h = screen->height;
  uint8_t channels = screen->channels;

  for (uint32_t k = 0; k < screen_size; k += channels) {
    uint32_t i = k / (channels * w);
    uint32_t j = (k / channels) % w;
    uint32_t x = j;
    uint32_t y = h - 1 - i;
    uint8_t *color = lambda(screen, x, y, context);
    if (color == NULL)
      continue;
    screen->canvas[k] = color[2];     // Blue
    screen->canvas[k + 1] = color[1]; // Green
    screen->canvas[k + 2] = color[0]; // Red
    if (channels == 4) {
      screen->canvas[k + 3] = MAX_BYTE; // Alpha (if present)
    }
    free(color);
  }
  // 	Reset file pointer to start of framebuffer
  lseek(screen->frame_buffer_fd, 0, SEEK_SET);
  // Push whole canvas to framebuffer in one call
  write(screen->frame_buffer_fd, screen->canvas, screen->size);

  return screen;
}



typedef struct time_struct {
  double time;
} Time;

uint8_t *anime(Screen *screen, uint32_t x, uint32_t y, void *context) {
  uint8_t *ans = malloc(sizeof(uint8_t) * 3);
  double time = ((Time *)context)->time;

  double px = (double)x * time / screen->width;
  double py = (double)y * time / screen->height;

  ans[0] = ((uint8_t)(MAX_BYTE * px)) % MAX_BYTE;
  ans[1] = ((uint8_t)(MAX_BYTE * py)) % MAX_BYTE;
  ans[2] = (uint8_t)0;

  return ans;
}

int main() {
  Screen *screen = new_screen();
  double t = 0;

  while (t < 10.0) {
    Time time = {.time = t};
    map(screen, anime, &time);
    t += 0.01;
  }

  close_screen(screen);
  return 0;
}
