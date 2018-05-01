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
    double x, y, z;
} Vect3d;

typedef struct {
    double x, y, z, w;
} Vect4d;

typedef struct {
    Vect3d pos;
    double yaw, pitch;
} Camera;

extern Camera camera;

void renderer_init(int argc, char **pString, char* title);
void renderer_start(void *data, void (*callback_step)(void *), void (*callback_draw)(void *));

#endif
