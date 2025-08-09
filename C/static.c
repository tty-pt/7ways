#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BYTE 8
#define MAX_BYTE 255

typedef struct {
  uint32_t size;
  uint32_t width;
  uint32_t height;
  uint8_t *canvas;
  uint8_t channels;
  uint8_t bits_per_pixel;
  int frame_buffer_fd;
} Screen;

typedef uint8_t *lambda_t(Screen *s, uint32_t x, uint32_t y);

Screen *Screen_new() {
  struct fb_var_screeninfo vinfo;
  int fb_fd = open("/dev/fb0", O_RDWR);
  if (fb_fd == -1) {
    perror("Error: cannot open framebuffer device");
    exit(1);
  }

  if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
    perror("Error reading variable information");
    exit(2);
  }
  int w = vinfo.xres;
  int h = vinfo.yres;
  printf("width: %d, height: %d", w, h);
  int color_channels = vinfo.bits_per_pixel / BYTE;
  long screen_size = w * h * color_channels;
  uint8_t *buffer = (uint8_t *)mmap(0, screen_size, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fb_fd, 0);
  Screen *ans = malloc(sizeof(Screen));
  ans->width = w;
  ans->height = h;
  ans->canvas = buffer;
  ans->size = screen_size;
  ans->channels = color_channels;
  ans->bits_per_pixel = vinfo.bits_per_pixel;
  ans->frame_buffer_fd = fb_fd;
  return ans;
}

void Screen_close(Screen *screen) {
  munmap(screen->canvas, screen->size);
  close(screen->frame_buffer_fd);
}

Screen *map(Screen *screen, lambda_t *lambda) {

  uint32_t screen_size = screen->size;
  uint32_t w = screen->width;
  uint32_t h = screen->height;
  uint8_t channels = screen->channels;
  uint8_t bpp = screen->bits_per_pixel;
  for (uint32_t k = 0; k < screen_size; k += channels) {
    uint32_t i = k / (channels * w);
    uint32_t j = (k / channels) % w;
    uint32_t x = j;
    uint32_t y = h - 1 - i;
    uint8_t *color = lambda(screen, x, y);
    if (color == NULL)
      continue;
    if (bpp == 32) {
      // BGRA
      screen->canvas[k] = color[2];
      screen->canvas[k + 1] = color[1];
      screen->canvas[k + 2] = color[0];
      screen->canvas[k + 3] = MAX_BYTE;
    } else if (bpp == 24) {
      // BGR
      screen->canvas[k] = color[2];
      screen->canvas[k + 1] = color[1];
      screen->canvas[k + 2] = color[0];
    } else if (bpp == 16) {
      // RGB565
      // 16 bits per pxl
      // fbset command shows what the 16 bit represents
      // rrrrrggggggbbbbb
      uint16_t b =
          (color[2] >> 3) & 0x1F; // == 32 * (color[2]/MAX_BYTE) & 0x1F;
      uint16_t g =
          (color[1] >> 2) & 0x3F; // == 64 * (color[1]/MAX_BYTE) & 0x3F;
      uint16_t r =
          (color[0] >> 3) & 0x1F; // == 32 * (color[0]/MAX_BYTE) & 0x1F;
      uint16_t c16 = (r << 11) | (g << 5) | b;
      screen->canvas[k] = c16 & 0xFF;
      screen->canvas[k + 1] = (c16 >> 8) & 0xFF;
    }
  }
  return screen;
}

uint8_t *shader(Screen *screen, uint32_t x, uint32_t y) {
  static uint8_t ans[3]; // maintain static storage
  ans[0] = (uint8_t)(MAX_BYTE * ((float)x / screen->width));
  ans[1] = (uint8_t)(MAX_BYTE * ((float)y / screen->height));
  ans[2] = 0;
  return ans;
}

int main() {
  Screen *screen = Screen_new();
  map(screen, shader);
  Screen_close(screen);
  return 0;
}
