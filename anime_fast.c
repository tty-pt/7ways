#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define BYTE 8
#define MAX_BYTE 255

typedef struct screen_struct
{
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint8_t *buffer;
    uint8_t *canvas;
    uint8_t channels;
    int frame_buffer_fd;
} Screen;

void close_screen(Screen *screen)
{
    munmap(screen->buffer, screen->size);
    close(screen->frame_buffer_fd);
}

Screen *anime(Screen *screen, double t)
{
    int k;
    uint32_t screen_size = screen->size;
    uint32_t w = screen->width;
    uint32_t h = screen->height;
    uint8_t channels = screen->channels;
    for (k = 0; k < screen_size; k += channels)
    {
        uint32_t i = k / (channels * w);
        uint32_t j = (k / channels) % w;
        uint32_t x = j;
        uint32_t y = h - 1 - i;
        double px = (double)x * t / screen->width;
        double py = (double)y * t / screen->height;
        uint8_t r = (uint8_t)(MAX_BYTE * fmod(px, 1.0));
        uint8_t g = (uint8_t)(MAX_BYTE * fmod(py, 1.0));
        uint8_t b = (uint8_t)0;
        screen->canvas[k] = b;
        screen->canvas[k + 1] = g;
        screen->canvas[k + 2] = r;
        screen->canvas[k + 3] = MAX_BYTE;
    }
    memcpy(screen->buffer, screen->canvas, screen->size);
    return screen;
}

Screen *new_screen()
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1)
    {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(2);
    }
    int w = vinfo.xres;
    int h = vinfo.yres;
    printf("width:%d, height: %d", w, h);
    int color_channels = (vinfo.bits_per_pixel / BYTE);
    long screen_size = w * h * color_channels;
    uint8_t *buffer = (uint8_t *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    uint8_t *canvas = (uint8_t *)malloc(sizeof(uint8_t) * screen_size);
    Screen *ans = malloc(sizeof(Screen));
    ans->width = w;
    ans->height = h;
    ans->buffer = buffer;
    ans->canvas = canvas;
    ans->size = screen_size;
    ans->channels = color_channels;
    ans->frame_buffer_fd = fb_fd;
    return ans;
}

int main()
{
    Screen *screen = new_screen();
    double t = 0;
    while (t < 10)
    {
        anime(screen, t);
        t = t + 0.05;
        usleep(50);
    }
    close_screen(screen);
    return 0;
}
