#include "../include/draw.h"

#include <qsys.h>

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static Display *g_dpy;
static Window   g_win;
static GLXContext g_ctx;
static GLuint   g_tex;

static void
glx_create_window(uint32_t w, uint32_t h)
{
	int att[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };

	g_dpy = XOpenDisplay(NULL);
	CBUG(!g_dpy, "XOpenDisplay");

	XVisualInfo *vi = glXChooseVisual(g_dpy,
			DefaultScreen(g_dpy), att);
	CBUG(!vi, "glXChooseVisual");

	Colormap cmap = XCreateColormap(g_dpy,
			RootWindow(g_dpy, vi->screen),
			vi->visual, AllocNone);

	XSetWindowAttributes swa = {0};
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | StructureNotifyMask;

	g_win = XCreateWindow(g_dpy,
			RootWindow(g_dpy, vi->screen),
			0, 0, w, h, 0, vi->depth,
			InputOutput, vi->visual,
			CWColormap | CWEventMask, &swa);

	XStoreName(g_dpy, g_win, "be (OpenGL)");
	XMapWindow(g_dpy, g_win);

	g_ctx = glXCreateContext(g_dpy, vi, 0, True);
	CBUG(!g_ctx, "glXCreateContext");
	glXMakeCurrent(g_dpy, g_win, g_ctx);

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (GLdouble) w, 0, (GLdouble) h, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
}

static void
gl_create_texture(uint32_t w, uint32_t h, const void *pixels)
{
	glGenTextures(1, &g_tex);
	glBindTexture(GL_TEXTURE_2D, g_tex);

	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA,
			(GLsizei)w, (GLsizei)h, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, pixels);
}

void be_init(void) {
	const char *env_w = getenv("BE_W");
	const char *env_h = getenv("BE_H");

	be_width = env_w
		? (uint32_t) strtoul(env_w, NULL, 10) : 1280;
	be_height = env_h
		? (uint32_t) strtoul(env_h, NULL, 10) : 720;

	glx_create_window(be_width, be_height);

	screen.channels = 4;
	screen.size = be_width * be_height;
	screen.canvas = (uint8_t*) calloc(
			(size_t) screen.size,
			screen.channels);

	CBUG(!screen.canvas, "calloc");

	screen.min_x = UINT_MAX;
	screen.min_y = UINT_MAX;
	screen.max_y = 0;

	gl_create_texture(be_width, be_height, screen.canvas);
}

void be_flush(void) {
	while (XPending(g_dpy)) {
		XEvent ev; XNextEvent(g_dpy, &ev);
		if (ev.type == ConfigureNotify) {
			// se quiseres suportar resize real:
			// re-aloca canvas e recria textura (omitido para simplicidade)
		}
	}

	glBindTexture(GL_TEXTURE_2D, g_tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			(GLsizei)be_width, (GLsizei)be_height,
			GL_BGRA, GL_UNSIGNED_BYTE, screen.canvas);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((float) be_width, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, (float) be_height);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((float) be_width, (float) be_height);
	glEnd();

	glXSwapBuffers(g_dpy, g_win);

	screen.min_x = UINT_MAX;
	screen.min_y = UINT_MAX;
	screen.max_y = 0;
}

void be_deinit(void) {
	glXMakeCurrent(g_dpy, None, NULL);
	glDeleteTextures(1, &g_tex);
	glXDestroyContext(g_dpy, g_ctx);
	XDestroyWindow(g_dpy, g_win);
	XCloseDisplay(g_dpy);

	free(screen.canvas);
	memset(&screen, 0, sizeof(screen));
}
