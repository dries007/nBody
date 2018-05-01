#include "renderer.h"

Camera camera = {
        (Vect3d){5, 5, 10},
        -30, -5
};
static void* callback_data;
static void (*callback_step_func)(void *);
static void (*callback_draw_func)(void *);
static bool mouseLeftDown = false;
static bool mouseRightDown = false;
static bool mouseMiddleDown = false;
static double mouseZoomDiv = 10;
static double mouseRotateDiv = 2.5;
static double mousePanDiv = 10;
static Vect2i prevMouse = {0, 0};

/* Window reshaped callback */
static void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, (double) w / h, 0.1, 1000);
    glViewport(0, 0, w, h);
}

static void drawAxis(double size)
{
    glPushMatrix();
    glPushAttrib(GL_LINE_STIPPLE);
    glPushAttrib(GL_LINE_STIPPLE_PATTERN);
    glPushAttrib(GL_LINE_STIPPLE_REPEAT);

    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3d(0, 0, 0); glVertex3d(size, 0, 0);
    glColor3f(0, 1, 0); glVertex3d(0, 0, 0); glVertex3d(0, size, 0);
    glColor3f(0, 0, 1); glVertex3d(0, 0, 0); glVertex3d(0, 0, size);
    glEnd();

    glLineStipple((int) (size / 100), 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3d(0, 0, 0); glVertex3d(-size, 0, 0);
    glColor3f(0, 1, 0); glVertex3d(0, 0, 0); glVertex3d(0, -size, 0);
    glColor3f(0, 0, 1); glVertex3d(0, 0, 0); glVertex3d(0, 0, -size);
    glEnd();

    glPopAttrib();/* GL_LINE_STIPPLE_REPEAT */
    glPopAttrib();/* GL_LINE_STIPPLE_PATTERN */
    glPopAttrib();/* GL_LINE_STIPPLE */
    glPopMatrix();
}

static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /* Nuke everything */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRotated(-camera.pitch, 1, 0, 0);
    glRotated(camera.yaw, 0, 1, 0);
    glTranslated(-camera.pos.x, -camera.pos.y, -camera.pos.z);

    drawAxis(10);
//    drawCheckers(1, 100);

    callback_draw_func(callback_data);

    glutSwapBuffers();
}

static void idle()
{
    callback_step_func(callback_data);
    glutPostRedisplay();
}

static void moveCamera(double forwards, double strafe, double yaw, double pitch, double x, double y, double z)
{
    if (forwards != 0)
    {
        double dz = -cos(camera.yaw*M_PI/180.0);
        double dy = sin(camera.pitch*M_PI/180.0);
        double dx = sin(camera.yaw*M_PI/180.0);
        camera.pos.x += dx * forwards;
        camera.pos.y += dy * forwards;
        camera.pos.z += dz * forwards;
    }
    if (strafe != 0)
    {
        double dz = sin(camera.yaw*M_PI/180.0);
        double dx = cos(camera.yaw*M_PI/180.0);
        camera.pos.x += dx * strafe;
        camera.pos.z += dz * strafe;
    }
    if (yaw != 0)
    {
        camera.yaw += yaw;
        if (camera.yaw > 180) camera.yaw -= 360;
        if (camera.yaw < -180) camera.yaw += 360;
    }
    if (pitch != 0)
    {
        camera.pitch += pitch;
        if (camera.pitch > 90) camera.pitch = 90;
        if (camera.pitch < -90) camera.pitch = -90;
    }
    if (x != 0 || y != 0 || z != 0)
    {
        camera.pos.x += x;
        camera.pos.y += y;
        camera.pos.z += z;
    }
    glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
    prevMouse.x = x;
    prevMouse.y = y;

    switch (button)
    {
        default: return;
        case GLUT_LEFT_BUTTON: mouseLeftDown = state == GLUT_DOWN; break;
        case GLUT_RIGHT_BUTTON: mouseRightDown = state == GLUT_DOWN; break;
        case GLUT_MIDDLE_BUTTON: mouseMiddleDown = state == GLUT_DOWN; break;
        case 3:
        case 4:
            // Scroll wheel
            moveCamera(button == 3 ? 1 : -1, 0, 0, 0, 0, 0, 0); // zoom
    }
}

static void motion(int x, int y)
{
    double dx = prevMouse.x - x;
    double dy = prevMouse.y - y;
    prevMouse.x = x;
    prevMouse.y = y;

    if (mouseLeftDown) moveCamera(0, dx/mouseZoomDiv, 0, 0, 0, dy/mousePanDiv, 0); // Y & strafe
    if (mouseRightDown) moveCamera(0, 0, -dx/mouseRotateDiv, dy/mouseRotateDiv, 0, 0, 0); // yaw & pitch
    if (mouseMiddleDown) moveCamera(0, 0, 0, 0, dx/mousePanDiv, 0, dy/mousePanDiv); // pan X Z
}

void renderer_init(int argc, char **argv, char* title)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA);
    glutInitWindowSize(900 * 16/9, 900);
    glutCreateWindow(title);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);

    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSetCursor(GLUT_CURSOR_INFO);
}

void renderer_start(void *data, void (*callback_step)(void *), void (*callback_draw)(void *))
{
    callback_data = data;
    callback_step_func = callback_step;
    callback_draw_func = callback_draw;

    glutIdleFunc(idle);

    glutMainLoop();
}
