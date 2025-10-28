#include "../include/draw.h"
#include <ttypt/qsys.h>
#include <ttypt/qmap.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GLFW/glfw3.h>

#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#else
# include <GL/gl.h>
# include <GL/glext.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

GLFWwindow *g_win;
static GLuint g_tex, g_fbo, g_tex_cpu;
static uint32_t g_tex_map_hd;

typedef struct {
	GLuint id;
	uint32_t w, h;
} gl_tex_info_t;

typedef struct {
	uint32_t ref;
	int32_t x, y;
	uint32_t cx, cy, sw, sh, dw, dh, tint;
} hw_cmd_t;

#define MAX_HW_CMDS	4096
static hw_cmd_t g_cmds[MAX_HW_CMDS];
static size_t g_ncmds;

static void glx_create_window(uint32_t w, uint32_t h)
{
	if (!glfwInit())
		exit(1);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	g_win = glfwCreateWindow((int)w, (int)h, "be (OpenGL)", NULL, NULL);
	if (!g_win)
		exit(2);

	glfwMakeContextCurrent(g_win);
	glfwSwapInterval(1);

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
}

static void gl_make_tex(GLuint *out, uint32_t w, uint32_t h)
{
	glGenTextures(1, out);
	glBindTexture(GL_TEXTURE_2D, *out);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
		     GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

void be_init(void)
{
	const char *env_w = getenv("BE_W");
	const char *env_h = getenv("BE_H");

	be_width  = env_w ? strtoul(env_w, NULL, 10) : 1280;
	be_height = env_h ? strtoul(env_h, NULL, 10) : 720;

	glx_create_window(be_width, be_height);

	glClearColor(0, 0, 0, 1);

	screen.channels = 4;
	screen.size = be_width * be_height;
	screen.canvas = calloc(screen.size, screen.channels);
	CBUG(!screen.canvas, "calloc");

	/* target do FBO */
	gl_make_tex(&g_tex, be_width, be_height);
	/* fonte (CPU canvas) */
	gl_make_tex(&g_tex_cpu, be_width, be_height);

	glGenFramebuffers(1, &g_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, g_tex, 0);
	CBUG(glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	     GL_FRAMEBUFFER_COMPLETE, "FBO incomplete");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	{
		uint32_t qm_gl_tex = qmap_reg(sizeof(gl_tex_info_t));
		g_tex_map_hd = qmap_open(NULL, NULL, QM_HNDL, qm_gl_tex, 0xF, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4ub(255, 255, 255, 255);
}

static inline void draw_fullscreen_quad(uint32_t w, uint32_t h)
{
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 1);
	glVertex2f(0, 0);
	glTexCoord2f(1, 1);
	glVertex2f(w, 0);
	glTexCoord2f(0, 0);
	glVertex2f(0, h);
	glTexCoord2f(1, 0);
	glVertex2f(w, h);
	glEnd();
}

static inline void flip_rows_inplace(uint8_t *p, uint32_t w, uint32_t h,
				     uint32_t bpp)
{
	size_t row = (size_t)w * bpp;
	uint8_t *tmp = (uint8_t *)malloc(row);
	uint32_t y;

	if (!tmp)
		return;

	for (y = 0; y < h / 2; y++) {
		uint8_t *a = p + (size_t)y * row;
		uint8_t *b = p + (size_t)(h - 1 - y) * row;

		memcpy(tmp, a, row);
		memcpy(a, b, row);
		memcpy(b, tmp, row);
	}

	free(tmp);
}

void be_flush(void)
{
	/* 1) Upload do canvas CPU -> g_tex_cpu (fonte) */
	glBindTexture(GL_TEXTURE_2D, g_tex_cpu);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, be_width, be_height,
			GL_BGRA, GL_UNSIGNED_BYTE, screen.canvas);

	/* 2) Composição no FBO: base + overlays */
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glViewport(0, 0, be_width, be_height);
	glClear(GL_COLOR_BUFFER_BIT);

	/* base (REPLACE) */
	glDisable(GL_BLEND);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, g_tex_cpu);
	draw_fullscreen_quad(be_width, be_height);

	/* overlays (MODULATE + BLEND) */
	if (g_ncmds) {
		size_t i;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		for (i = 0; i < g_ncmds; i++) {
			const hw_cmd_t *c = &g_cmds[i];
			const gl_tex_info_t *tex =
				qmap_get(g_tex_map_hd, &c->ref);
			uint8_t ta, tr, tg, tb;
			float u0, v0, u1, v1;
			float x0, y0, x1, y1;

			if (!tex)
				continue;

			ta = c->tint >> 24;
			tr = c->tint >> 16;
			tg = c->tint >> 8;
			tb = c->tint;

			glColor4ub(tr, tg, tb, ta);
			glBindTexture(GL_TEXTURE_2D, tex->id);

			u0 = (float)c->cx / tex->w;
			v0 = (float)c->cy / tex->h;
			u1 = (float)(c->cx + c->sw) / tex->w;
			v1 = (float)(c->cy + c->sh) / tex->h;

			x0 = c->x;
			y0 = c->y;
			x1 = x0 + c->dw;
			y1 = y0 + c->dh;

			glBegin(GL_QUADS);
			glTexCoord2f(u0, v0);
			glVertex2f(x0, y0);
			glTexCoord2f(u1, v0);
			glVertex2f(x1, y0);
			glTexCoord2f(u1, v1);
			glVertex2f(x1, y1);
			glTexCoord2f(u0, v1);
			glVertex2f(x0, y1);
			glEnd();
		}

		glDisable(GL_BLEND);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor4ub(255, 255, 255, 255);
	}

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, be_width, be_height, GL_BGRA, GL_UNSIGNED_BYTE,
		     screen.canvas);

	/* Garantir canvas em top-first para a próxima frame: */
	flip_rows_inplace((uint8_t *)screen.canvas, be_width, be_height, 4);

	/* 3) Apresentação: desenhar o target do FBO (g_tex) no backbuffer */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, be_width, be_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, g_tex);
	draw_fullscreen_quad(be_width, be_height);

	glfwSwapBuffers(g_win);
	glfwPollEvents();
	g_ncmds = 0;
}

void be_deinit(void)
{
	const void *key, *val;
	uint32_t it;

	glDeleteTextures(1, &g_tex);
	glDeleteFramebuffers(1, &g_fbo);

	it = qmap_iter(g_tex_map_hd, NULL, 0);
	while (qmap_next(&key, &val, it))
		glDeleteTextures(1, &((gl_tex_info_t *)val)->id);

	qmap_close(g_tex_map_hd);

	glfwDestroyWindow(g_win);
	glfwTerminate();

	free(screen.canvas);
	memset(&screen, 0, sizeof(screen));
}

void be_render_img_ex(uint32_t ref, int32_t x, int32_t y,
		      uint32_t cx, uint32_t cy, uint32_t sw, uint32_t sh,
		      uint32_t dw, uint32_t dh, uint32_t tint)
{
	if (g_ncmds < MAX_HW_CMDS)
		g_cmds[g_ncmds++] = (hw_cmd_t) {
			.ref = ref,
			.x = x,
			.y = y,
			.cx = cx,
			.cy = cy,
			.sw = sw,
			.sh = sh,
			.dw = dw,
			.dh = dh,
			.tint = tint
		};
}

void be_register_texture(uint32_t ref, uint8_t *data,
			 uint32_t w, uint32_t h)
{
	gl_tex_info_t tex = { .w = w, .h = h };

	glGenTextures(1, &tex.id);
	glBindTexture(GL_TEXTURE_2D, tex.id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
		     GL_BGRA, GL_UNSIGNED_BYTE, data);

	qmap_put(g_tex_map_hd, &ref, &tex);
}

void be_unregister_texture(uint32_t ref)
{
	const gl_tex_info_t *t = qmap_get(g_tex_map_hd, &ref);

	if (t) {
		glDeleteTextures(1, &t->id);
		qmap_del(g_tex_map_hd, &ref);
	}
}

void be_update_texture_rect(uint32_t ref, uint32_t x, uint32_t y,
			    uint32_t w, uint32_t h, uint8_t *data)
{
	const gl_tex_info_t *t = qmap_get(g_tex_map_hd, &ref);

	if (!t)
		return;

	glBindTexture(GL_TEXTURE_2D, t->id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
			GL_RGBA, GL_UNSIGNED_BYTE, data);
}
