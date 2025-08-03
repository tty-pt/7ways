#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_BYTE 255
#define BYTE 8

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t size;
    uint8_t bytesPerPixel;
    uint8_t channels;
    uint8_t *buffer;
    int fb;
} Screen;

Screen *Screen_new()
{
    struct fb_var_screeninfo vinfo;
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
    uint32_t w = vinfo.xres;
    uint32_t h = vinfo.yres;
    uint8_t channels = vinfo.bits_per_pixel / BYTE;
    uint32_t size = w * h * channels;
    // We avoid mmap for double buffering simulation
    uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * size);
    if (!buffer)
    {
        perror("Failed to allocate canvas buffer");
        close(fb_fd);
        exit(3);
    }

    Screen *screen = malloc(sizeof(Screen));
    screen->width = w;
    screen->height = h;
    screen->bytesPerPixel = channels;
    screen->channels = channels;
    screen->size = size;
    screen->buffer = buffer;
    screen->fb = fb_fd;
    return screen;
}

void Screen_paint(Screen *screen)
{
    lseek(screen->fb, 0, SEEK_SET);
    write(screen->fb, screen->buffer, screen->size);
}

void Screen_map(Screen *screen, double time)
{
    uint32_t w = screen->width;
    uint32_t h = screen->height;
    uint8_t channels = screen->bytesPerPixel;

    for (uint32_t k = 0; k < screen->size; k += channels)
    {
        uint32_t i = k / (channels * w);
        uint32_t j = (k / channels) % w;
        uint32_t x = j;
        uint32_t y = h - 1 - i;
        double px = ((double)x * time) / w;
        double py = ((double)y * time) / h;
        uint8_t r = ((uint8_t)(MAX_BYTE * fmod(px, 1.0)));
        uint8_t g = ((uint8_t)(MAX_BYTE * fmod(py, 1.0)));
        uint8_t b = 0;
        screen->buffer[k] = b;
        screen->buffer[k + 1] = g;
        screen->buffer[k + 2] = r;
        screen->buffer[k + 3] = MAX_BYTE;
    }
    Screen_paint(screen);
}

void Screen_close(Screen *screen)
{
    free(screen->buffer);
    close(screen->fb);
    free(screen);
}

int main()
{
    Screen *screen = Screen_new();
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double start = ts.tv_sec + ts.tv_nsec * 1e-9;
    double time = 0;
    while (time < 10)
    {
        Screen_map(screen, time);
        // usleep(16 * 1000); // ~60 FPS
        clock_gettime(CLOCK_MONOTONIC, &ts);
        double now = ts.tv_sec + ts.tv_nsec * 1e-9;
        time = now - start;
    }
    Screen_close(screen);
    return 0;
}