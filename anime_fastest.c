// Standard I/O for printf, perror
#include <stdio.h>
// Standard library for malloc, free, exit
#include <stdlib.h>
// Fixed-width integer types (e.g., uint8_t, uint16_t)
#include <stdint.h>
// String operations (e.g., memcpy, memset)
#include <string.h>
// File control options (open, O_RDWR)
#include <fcntl.h>
// IO control for framebuffer operations
#include <sys/ioctl.h>
// Linux framebuffer definitions
#include <linux/fb.h>
// Memory mapping (mmap)
#include <sys/mman.h>
// Unix standard functions (close, usleep)
#include <unistd.h>
// Time functions (not used, but often for timing)
#include <time.h>
// Math functions (fmodf)
#include <math.h>

#define PI 3.14159265358979323846


int main() {
    // Open the framebuffer device for reading and writing
    int fb = open("/dev/fb0", O_RDWR);
    if (fb < 0) {
        perror("open /dev/fb0");
        return 1;
    }

    // Structures to hold framebuffer info
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    // Get fixed screen info
    if (ioctl(fb, FBIOGET_FSCREENINFO, &finfo)) {
        perror("ioctl FSCREENINFO");
        close(fb);
        return 1;
    }
    // Get variable screen info
    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("ioctl VSCREENINFO");
        close(fb);
        return 1;
    }

    // Extract screen parameters
    int width = vinfo.xres;
    int height = vinfo.yres;
    int bpp = vinfo.bits_per_pixel;
    int screensize = finfo.line_length * height;

    // Map framebuffer to memory
    uint8_t *fbp = (uint8_t *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (fbp == MAP_FAILED) {
        perror("mmap");
        close(fb);
        return 1;
    }

    // Allocate double buffer in memory for drawing
    uint8_t *buffer = (uint8_t *)malloc(screensize);
    if (!buffer) {
        perror("malloc buffer");
        munmap(fbp, screensize);
        close(fb);
        return 1;
    }

    float t = 0.0f;
    unsigned int frame_count = 0;
    while (1) {
        // Draw the frame into the memory buffer
        for (int y = 0; y < height; ++y) {
            float fy = 1.0f - ((float)y / height);
            for (int x = 0; x < width; ++x) {
                // Normalized coordinates
                float fx = (float)x / width;
                // Simple animated color gradient
                float r = fmodf(fx * t, 1.0f);
                float g = fmodf(fy * t, 1.0f);
                float b = 0.0f;
                // Convert to 8-bit color
                uint8_t R = (uint8_t)(r * 255);
                uint8_t G = (uint8_t)(g * 255);
                uint8_t B = (uint8_t)(b * 255);
                // Calculate pixel location in buffer
                int location = y * finfo.line_length + x * (bpp/8);
                if (bpp == 32) {
                    // 32bpp: BGRA
                    buffer[location+0] = B;
                    buffer[location+1] = G;
                    buffer[location+2] = R;
                    buffer[location+3] = 0;
                } else if (bpp == 24) {
                    // 24bpp: BGR
                    buffer[location+0] = B;
                    buffer[location+1] = G;
                    buffer[location+2] = R;
                } else if (bpp == 16) {
                    // 16bpp: RGB565
                    uint16_t pixel = ((R>>3)<<11) | ((G>>2)<<5) | (B>>3);
                    ((uint16_t*)buffer)[y*width + x] = pixel;
                }
            }
        }
        // Copy the memory buffer to the framebuffer in one operation
        memcpy(fbp, buffer, screensize);
        t += 0.03f;
        frame_count++;
    }

    // Free resources (unreachable in this loop, but good practice)
    free(buffer);
    munmap(fbp, screensize);
    close(fb);
    return 0;
}