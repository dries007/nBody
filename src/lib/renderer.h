#ifndef _RENDERER_H_
#define _RENDERER_H_

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
        struct {float x, y, z;};
        struct {float r, g, b;};
    };
} Vect3f;

typedef struct {
    union {
        struct {double x, y, z, w;};
        struct {double r, g, b, a;};
    };
} Vect4d;

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

extern Camera camera;

void renderer_init(int argc, char **pString, char* title);
void renderer_start(void *data, void (*callback_step)(void *), void (*callback_draw)(void *));

#endif
