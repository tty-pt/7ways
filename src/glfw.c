#include "../include/draw.h"
#include "../include/gl.h"

#include <stdlib.h>

#include <GLFW/glfw3.h>

#include <ttypt/qsys.h>

GLFWwindow *g_win;

extern void gl_init(uint32_t w, uint32_t h);
extern void gl_flush(void);
extern void gl_deinit(void);
extern GLuint g_tex;

void be_init(void)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

	g_win = glfwCreateWindow(1280, 720, "be", NULL, NULL);
	CBUG(!g_win, "glfwCreateWindow");

	glfwMakeContextCurrent(g_win);
	glfwSwapInterval(1);

	glfwGetFramebufferSize(g_win, (int32_t *) &be_width, (int32_t *) &be_height);

#ifndef __APPLE__
	CBUG(glewInit() != GLEW_OK, "glfwCreateWindow");
#endif

	gl_init(be_width, be_height);
}


void be_flush(void)
{
	gl_flush();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, be_width, be_height);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, g_tex);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0,1); glVertex2f(0, be_height);
	glTexCoord2f(1,1); glVertex2f(be_width, be_height);
	glTexCoord2f(0,0); glVertex2f(0, 0);
	glTexCoord2f(1,0); glVertex2f(be_width, 0);
	glEnd();

	glfwSwapBuffers(g_win);
	glfwPollEvents();
}

void be_deinit(void)
{
	glfwDestroyWindow(g_win);
	glfwTerminate();
}
