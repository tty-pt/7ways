#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>

#define BYTE 8
#define MAX_BYTE 255

int main()
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
    int color_channels = (vinfo.bits_per_pixel / BYTE);
    long screen_size = w * h * color_channels;
    uint8_t *buffer = (uint8_t *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    uint8_t *canvas = (uint8_t *)malloc(sizeof(uint8_t) * screen_size);

    double t = 0;
    while (t < 10)
    {
        long k;
        for (k = 0; k < screen_size; k += color_channels)
        {
            uint32_t i = k / (color_channels * w);
            uint32_t j = (k / color_channels) % w;
            uint32_t x = j;
            uint32_t y = h - 1 - i;
            double px = (double)x * t / w;
            double py = (double)y * t / h;
            uint8_t r = (uint8_t)(MAX_BYTE * px) % MAX_BYTE;
            uint8_t g = (uint8_t)(MAX_BYTE * py) % MAX_BYTE;
            uint8_t b = (uint8_t)0;
            canvas[k] = b;
            canvas[k + 1] = g;
            canvas[k + 2] = r;
            canvas[k + 3] = MAX_BYTE;
        }
        memcpy(buffer, canvas, screen_size);
        t = t + 0.05;
        usleep(50);
    }
    munmap(buffer, screen_size);
    close(fb_fd);
    return 0;
}
