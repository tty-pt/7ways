#include "../include/draw.h"

#include <ttypt/qsys.h>

#include <GLFW/glfw3.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#else
#  include <GL/gl.h>
#  include <GL/glext.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


GLFWwindow *g_win;
static GLuint   g_tex;

static void
glx_create_window(uint32_t w, uint32_t h)
{
	if (!glfwInit()) {
		fprintf(stderr, "glfwInit failed\n");
		exit(1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	g_win = glfwCreateWindow((int)w, (int)h, "be (OpenGL)", NULL, NULL);
	if (!g_win) {
		fprintf(stderr, "glfwCreateWindow failed\n");
		glfwTerminate();
		exit(2);
	}

	glfwMakeContextCurrent(g_win);
	glfwSwapInterval(1); // VSync

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (GLdouble)w, 0, (GLdouble)h, -1, 1);

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
             (GLsizei) w, (GLsizei) h, 0,
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
	/*
	while (XPending(g_dpy)) {
		XEvent ev; XNextEvent(g_dpy, &ev);
		if (ev.type == ConfigureNotify) {
			// se quiseres suportar resize real:
			// re-aloca canvas e recria textura (omitido para simplicidade)
		}
	}
	*/

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

	glfwSwapBuffers(g_win);
	glfwPollEvents();

	screen.min_x = UINT_MAX;
	screen.min_y = UINT_MAX;
	screen.max_y = 0;
}

void be_deinit(void) {
	glDeleteTextures(1, &g_tex);
	glfwDestroyWindow(g_win);
	glfwTerminate();

	free(screen.canvas);
	memset(&screen, 0, sizeof(screen));
}
