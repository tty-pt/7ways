#ifndef QG_GL_H
#define QG_GL_H

#define GL_GLEXT_PROTOTYPES 1
#define GLEW_STATIC

#ifdef __APPLE__
# define GL_SILENCE_DEPRECATION
# include <OpenGL/glext.h>
# include <OpenGL/gl.h>
#else
# include <GL/glew.h>
# include <GL/gl.h>
#endif

extern GLuint g_tex;

void gl_init(uint32_t w, uint32_t h);
void gl_deinit(void);
void gl_flush(void);
void gl_fullscreen_quad(uint32_t w, uint32_t h);
void gl_img_ureg(uint32_t ref);

#endif
