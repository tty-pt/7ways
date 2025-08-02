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
    int color_channels = (vinfo.bits_per_pixel / BYTE);
    long screen_size = w * h * color_channels;
    uint8_t *buffer = (uint8_t *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Error: mmap failed");
        close(fb_fd);
        exit(3);
    }

    double t = 0;
    while (t < 10)
    {
        double t_over_w = t / w;
        double t_over_h = t / h;
        for (uint32_t i = 0; i < h; ++i)
        {
            uint32_t y = h - 1 - i;
            uint8_t g = (uint8_t)(MAX_BYTE * y * t_over_h);

            for (uint32_t j = 0; j < w; ++j)
            {
                uint8_t r = (uint8_t)(((int)(MAX_BYTE * j * t_over_w)) % 256);
                uint8_t b = 0;
                long k = (i * w + j) * color_channels;
                buffer[k]     = b;
                buffer[k + 1] = g;
                buffer[k + 2] = r;
                buffer[k + 3] = MAX_BYTE;
            }
        }
        t += 0.01;
    }
    munmap(buffer, screen_size);
    close(fb_fd);
    return 0;
}
