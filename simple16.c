#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdint.h>

#define BYTE 8
#define MAX_BYTE 255

typedef struct screen_struct
{
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint8_t *buffer;
    uint8_t bits_per_pxl;
    uint8_t channels;
    int frame_buffer_fd;
} Screen;

void close_screen(Screen *screen)
{
    munmap(screen->buffer, screen->size);
    close(screen->frame_buffer_fd);
}

Screen *map(Screen *screen, uint8_t *(*lambda)(Screen *s, uint32_t x, uint32_t y))
{
    uint32_t k;
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
        uint8_t *color = lambda(screen, x, y);
        if (color == NULL)
            continue;
        // 16 bits per pxl
        // fbset command shows what the 16 bit represents
        // rrrrrggggggbbbbb
        uint16_t b = (color[2] >> 3) & 0x1F; // == 32 * (color[2]/MAX_BYTE) & 0x1F;
        uint16_t g = (color[1] >> 2) & 0x3F; // == 64 * (color[1]/MAX_BYTE) & 0x3F;
        uint16_t r = (color[0] >> 3) & 0x1F; // == 32 * (color[0]/MAX_BYTE) & 0x1F;
        uint16_t c16 = (r << 11) | (g << 5) | b;
        screen->buffer[k] = c16 & 0xFF;
        screen->buffer[k + 1] = (c16 >> 8) & 0xFF;
        free(color);
    }
    return screen;
}

Screen *new_screen()
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
    int w = vinfo.xres;
    int h = vinfo.yres;
    int color_channels = vinfo.bits_per_pixel / BYTE;
    long screen_size = w * h * color_channels;
    uint8_t *buffer = (uint8_t *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    Screen *ans = malloc(sizeof(Screen));
    ans->width = w;
    ans->height = h;
    ans->buffer = buffer;
    ans->size = screen_size;
    ans->channels = color_channels;
    ans->bits_per_pxl = vinfo.bits_per_pixel;
    ans->frame_buffer_fd = fb_fd;
    return ans;
}

uint8_t *shader(Screen *screen, uint32_t x, uint32_t y)
{
    uint8_t *ans = malloc(sizeof(uint8_t) * 3);
    ans[0] = MAX_BYTE * ((float)x / screen->width);
    ans[1] = MAX_BYTE * ((float)y / screen->height);
    ans[2] = 0;
    return ans;
}

int main()
{
    Screen *screen = new_screen();
    map(screen, shader);
    close_screen(screen);
    return 0;
}
