#define DECLARE_GLOBALS
#include "renderer.h"
#include "camera.h"

#ifdef __APPLE__
# include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

static void create_shader_program();
static void check_shader_compile(GLuint shader);

static GLuint g_vao;
static GLuint g_vbo;

char *read_shader_source(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Failed to read file");
        exit(-1);
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *code = (char *)malloc(file_size + 1);
    fread(code, file_size, 1, fp);
    fclose(fp);
    code[file_size] = 0;
    return code;
}

// Based on: https://www.khronos.org/opengl/wiki/Tutorial1:_Creating_a_Cross_Platform_OpenGL_3.2_Context_in_SDL_(C_/_SDL)
void init_gl()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) /* Initialize SDL's Video subsystem */
    {
        printf("Unable to initialize SDL context\n");
        SDL_Quit();
        exit(1);
    }
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //sdl_err();


    /* Create our window centered at 512x512 resolution */
    g_gl_window = SDL_CreateWindow("n-body", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!g_gl_window) /* Die if creation failed */
    {
        printf("Window creation failed\n");
        SDL_Quit();
        exit(1);
    }

    //sdl_err();

    /* Create our opengl context and attach it to our window */
    g_gl_context = SDL_GL_CreateContext(g_gl_window);
    //sdl_err();


    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);

    printf("Creating shader program...\n");
    create_shader_program();
    //SDL_CaptureMouse(1);

}

void check_shader_compile(GLuint shader)
{
    int is_compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
    if (!is_compiled)
    {
        int max_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

       /* The maxLength includes the NULL character */
       char* info_log = (char *)malloc(max_length);

       glGetShaderInfoLog(shader, max_length, &max_length, info_log);

       printf("ERROR: Failed to compile shader: \n");
       printf("%s\n", info_log);

       /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
       /* In this simple program, we'll just leave */
       free(info_log);
       exit(1);
    }
}


void create_shader_program()
{
    char* vertex_shader_source = read_shader_source("n-body.vert");
    char* fragment_shader_source = read_shader_source("n-body.frag");

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar**)&vertex_shader_source, 0);
    glCompileShader(vertex_shader);
    check_shader_compile(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const GLchar**)&fragment_shader_source, 0);
    glCompileShader(fragment_shader);
    check_shader_compile(fragment_shader);

    g_shader_program = glCreateProgram();
    glAttachShader(g_shader_program, vertex_shader);
    glAttachShader(g_shader_program, fragment_shader);

    glBindAttribLocation(g_shader_program, 0, "in_Position");

    glLinkProgram(g_shader_program);

    int is_linked;
    glGetProgramiv(g_shader_program, GL_LINK_STATUS, &is_linked);
    if(!is_linked)
    {
       int max_length;
       glGetProgramiv(g_shader_program, GL_INFO_LOG_LENGTH, &max_length);

       /* The maxLength includes the NULL character */
       char* info_log = (char *)malloc(max_length);

       /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
       glGetProgramInfoLog(g_shader_program, max_length, &max_length, info_log);
       printf("ERROR: Failed to link program: \n");
       printf("%s\n", info_log);

       /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
       /* In this simple program, we'll just leave */
       free(info_log);
       exit(1);
    }

    glUseProgram(g_shader_program);
    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

    init_camera();

    glEnable(GL_DEPTH_TEST);
}

int render_point_cloud(float* point_cloud, int length)
{

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            printf("Quit");
            return 1;
        }

        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    return 1;
                case SDLK_w:
                    moveCamera(0.f, 0.f, 0.5f);
                    break;
                case SDLK_s:
                    moveCamera(0.f, 0.f, -0.5f);
                    break;
            }
        }
        if (event.type == SDL_MOUSEWHEEL)
        {
            if (event.wheel.y > 0.f)
                moveCamera(0.f, 0.f, 1.f);
            else
                moveCamera(0.f, 0.f, -1.f);
        }
    }

    int delta_x;
    int delta_y;

    SDL_GetRelativeMouseState(&delta_x, &delta_y);

    rotateCamera(delta_x, delta_y);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

    glBufferData(GL_ARRAY_BUFFER, length * sizeof(cl_float3), point_cloud, GL_DYNAMIC_DRAW);

    GLuint matrix_id = glGetUniformLocation(g_shader_program, "mvp_mat");
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, getMVPMat());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cl_float3), 0);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnableVertexAttribArray(0);


    glDrawArrays(GL_POINTS, 0, length);

    SDL_GL_SwapWindow(g_gl_window);

    return 0;

}


void deinit_gl()
{
    /* Delete our opengl context, destroy our window, and shutdown SDL */
    //SDL_GL_DeleteContext(g_gl_context);
    //SDL_DestroyWindow(g_gl_window);
    //SDL_Quit();

}
