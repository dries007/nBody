#ifndef _RENDERER_H_
#define _RENDERER_H_
/**
 * (c) Dries Kennes 2018
 *
 * Rendering helper. Largely a copy of my personal OpenGL project.
 */
/* Handy includes for everything */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <GL/glut.h>

/* Types */
typedef struct {
    int x, y;
} Vect2i;

typedef struct {
    union {
        struct {double x, y, z;};
        struct {double r, g, b;};
    };
} Vect3d;

typedef struct {
    union {
        struct {float x, y, z, w;};
        struct {float r, g, b, a;};
    };
} Vect4f;

typedef struct {
    Vect3d pos;
    double yaw, pitch;
} Camera;

/* Public global variables */
extern Camera camera;

/* Public 'API' */
void renderer_init(int argc, char **pString, char* title);
void renderer_start(void *data, void (*callback_step)(void *), void (*callback_draw)(void *));

#endif
