#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>


#ifndef DECLARE_GLOBALS
#define EXTERNAL extern
#else
#define EXTERNAL
#endif


EXTERNAL SDL_Window* g_gl_window;
EXTERNAL SDL_GLContext g_gl_context;
EXTERNAL GLuint g_shader_program;


void init_gl();
void deinit_gl();
int render_point_cloud(float* point_cloud, int length);


#define sdl_err() \
do {\
    const char *error = SDL_GetError();\
    if (*error != '\0')\
    {\
        fprintf(stderr, "SDL ERROR:\n\t"\
                        "Problem: %s\n\t"\
                        "Filename: %s\n\t"\
                        "Line: %d\n",\
                        error,\
                        __FILE__, __LINE__);\
        abort();\
    }\
} while (0)

#endif // _RENDERER_H_
