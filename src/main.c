/**
 * (c) Dries Kennes & Stijn Van Dessel 2018
 *
 * nBody problem (Main program)
 */
#include "lib/ocl_utils.h"
#include "lib/renderer.h"

#define AVG_MAX 100

enum MODE {
    MODE_CPU,
    MODE_FLOAT3,
    MODE_FLOAT,
    MODE_2D,
};

typedef struct body
{
    cl_float3 pos;
    cl_float3 speed;
} Body;

typedef struct callback_data {
    cl_kernel kernel;
    size_t n;
    Body* bodies;
    cl_mem dev_bodies;
    Vect4f point_color;
    float point_size;
    bool draw_lines;
    Vect4f line_color;
    enum MODE mode;
} CallbackData;

Body* create_bodies(size_t n)
{
    Body* data = calloc(n, sizeof(Body));
    if (data == NULL) abort();

    for (int i = 0; i < n; i++)
    {
        float offset = (rand() < RAND_MAX / 2) ? -5 : 5;
        data[i].pos.x = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f + offset;
        data[i].pos.y = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f;
        data[i].pos.z = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f;

        data[i].speed.x = ((float)rand() / (float)RAND_MAX) * 1.f - 0.5f;
        data[i].speed.y = ((float)rand() / (float)RAND_MAX) * 1.f - 0.5f;
        data[i].speed.z = ((float)rand() / (float)RAND_MAX) * 1.f - 0.5f;
    }

    return data;
}

void draw(void* data)
{
    CallbackData* cd = data;

    glColor4fv((const GLfloat *) &cd->point_color);
    glPointSize(cd->point_size);
    glBegin(GL_POINTS);
    for (int i = 0; i < cd->n; i++)
    {
        glVertex3fv((float *) &cd->bodies[i].pos);
    }
    glEnd();
    if (cd->draw_lines)
    {
        glColor4fv((float *) &cd->line_color);
        glBegin(GL_LINES);
        for (int i = 0; i < cd->n; i++)
        {
            glVertex3fv((float *) &cd->bodies[i].pos);
            glVertex3f(
                    cd->bodies[i].pos.x + cd->bodies[i].speed.x / 2,
                    cd->bodies[i].pos.y + cd->bodies[i].speed.y / 2,
                    cd->bodies[i].pos.z + cd->bodies[i].speed.z / 2);
        }
        glEnd();
    }
}

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

void kernel_cpu(CallbackData* cd)
{
    const float delta_time = 1.f;
    const float grav_constant = 1;
    const float mass_of_sun = 2;
    const float mass_grav = grav_constant * mass_of_sun * mass_of_sun;
    const float scale = 50;

    for (int i = 0; i < cd->n; ++i)
    {
        for (int j = 0; j < cd->n; ++j)
        {
            if (i == j) continue;

            cl_float3 pos_a = cd->bodies[i].pos;
            cl_float3 pos_b = cd->bodies[j].pos;

            float dist_x = (pos_a.s[0] - pos_b.s[0]) * scale;
            float dist_y = (pos_a.s[1] - pos_b.s[1]) * scale;
            float dist_z = (pos_a.s[2] - pos_b.s[2]) * scale;

            float distance = sqrt(
                    dist_x * dist_x +
                    dist_y * dist_y +
                    dist_z * dist_z);

            float force_x = -mass_grav * dist_x / (distance * distance * distance);
            float force_y = -mass_grav * dist_y / (distance * distance * distance);
            float force_z = -mass_grav * dist_z / (distance * distance * distance);

            float acc_x = force_x / mass_of_sun;
            float acc_y = force_y / mass_of_sun;
            float acc_z = force_z / mass_of_sun;

            cd->bodies[i].speed.s[0] += acc_x * delta_time;
            cd->bodies[i].speed.s[1] += acc_y * delta_time;
            cd->bodies[i].speed.s[2] += acc_z * delta_time;
        }
    }

    for (int i = 0; i < cd->n; ++i)
    {
        cd->bodies[i].pos.s[0] += (cd->bodies[i].speed.s[0] * delta_time) / scale;
        cd->bodies[i].pos.s[1] += (cd->bodies[i].speed.s[1] * delta_time) / scale;
        cd->bodies[i].pos.s[2] += (cd->bodies[i].speed.s[2] * delta_time) / scale;
    }
}

void step(void* data)
{
    static struct timespec avg[AVG_MAX];
    static int time_index = 0;

    struct timespec time1, time2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

    CallbackData* cd = data;

    switch (cd->mode)
    {
        case MODE_CPU:
            kernel_cpu(cd);
        break;
        case MODE_FLOAT3:
        case MODE_FLOAT:
            ocl_err(clEnqueueNDRangeKernel(g_command_queue, cd->kernel, 1, NULL, &cd->n, NULL,
                                           0, NULL, NULL));
            ocl_err(clFinish(g_command_queue));
            ocl_err(clEnqueueReadBuffer(g_command_queue, cd->dev_bodies, CL_TRUE, 0,
                                        sizeof(Body) * cd->n, cd->bodies, 0, NULL, NULL));
            ocl_err(clFinish(g_command_queue));
            break;
        case MODE_2D:
        {
            size_t size[] = {cd->n, cd->n};
            ocl_err(clEnqueueWriteBuffer(g_command_queue, cd->dev_bodies, CL_TRUE, 0,
                                         sizeof(Body) * cd->n, cd->bodies, 0, NULL, NULL));
            ocl_err(clFinish(g_command_queue));
            ocl_err(clEnqueueNDRangeKernel(g_command_queue, cd->kernel, 2, NULL, size, NULL,
                                           0, NULL, NULL));
            ocl_err(clFinish(g_command_queue));
            ocl_err(clEnqueueReadBuffer(g_command_queue, cd->dev_bodies, CL_TRUE, 0,
                                        sizeof(Body) * cd->n, cd->bodies, 0, NULL, NULL));
            ocl_err(clFinish(g_command_queue));
            const float SCALE = 50;
            const float TIME_DELTA = 1;
            for (int i = 0; i < cd->n; ++i)
            {
                cd->bodies[i].pos.s[0] += (cd->bodies[i].speed.s[0] * TIME_DELTA) / SCALE;
                cd->bodies[i].pos.s[1] += (cd->bodies[i].speed.s[1] * TIME_DELTA) / SCALE;
                cd->bodies[i].pos.s[2] += (cd->bodies[i].speed.s[2] * TIME_DELTA) / SCALE;
            }
        }
            break;
        default:
            abort();
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    avg[time_index] = diff(time1, time2);
    time_index ++;
    if (time_index == AVG_MAX)
    {
        time_index = 0;
        double sum = 0;
        for (int i = 0; i < AVG_MAX; i++) {
            sum += avg[i].tv_sec + avg[i].tv_nsec / 1000000000.0;
        }
        sum /= AVG_MAX;
        printf("AVG: %lf\n", sum);
    }
}

int main(int argc, char ** argv)
{
    if (argc <= 2)
    {
        printf("%s <number of bodies> <mode>\n", argv[0]);
        printf("Mode must be one of:\n");
        printf("- CPU\n");
        printf("- FLOAT\n");
        printf("- FLOAT3\n");
        printf("- 2D\n");
        return -1;
    }
    errno = 0;
    cl_ulong n = strtoul(argv[1], NULL, 0);
    if (errno != 0)
    {
        printf("'%s' was not converted to a proper number. (%ld)\n", argv[1], n);
        return -1;
    }
    enum MODE mode;
    if (strcmp(argv[2], "CPU") == 0 || strcmp(argv[2], "cpu") == 0)
    {
        mode = MODE_CPU;
    }
    else if (strcmp(argv[2], "FLOAT3") == 0 || strcmp(argv[2], "float3") == 0)
    {
        mode = MODE_FLOAT3;
    }
    else if (strcmp(argv[2], "FLOAT") == 0 || strcmp(argv[2], "float") == 0)
    {
        mode = MODE_FLOAT;
    }
    else if (strcmp(argv[2], "2D") == 0 || strcmp(argv[2], "2d") == 0)
    {
        mode = MODE_2D;
    }
    else
    {
        printf("'%s' was not converted to a mode.\n", argv[1]);
        return -1;
    }

    if (mode != MODE_CPU)
    {
        cl_platform_id platform = ocl_select_platform();
        cl_device_id device = ocl_select_device(platform);
        init_ocl(device);
        create_program("kernel.cl", "");
    }

    char* title = malloc(1 + (size_t) snprintf(NULL, 0, "nBody problem - %ld bodies", n));
    sprintf(title, "nBody problem - %ld bodies", n);
    renderer_init(argc, argv, title);

    srand((unsigned int) time(NULL));
    Body* bodies = create_bodies((size_t) n);

    // Create kernel
    cl_int error = 0;
    cl_kernel kernel = NULL;
    cl_mem dev_bodies;
    switch (mode)
    {
        case MODE_CPU:
            kernel = NULL;
            dev_bodies = NULL;
            break;
        case MODE_FLOAT3:
            kernel = clCreateKernel(g_program, "kernel_step_float3", &error);
            break;
        case MODE_FLOAT:
            kernel = clCreateKernel(g_program, "kernel_step_float", &error);
            break;
        case MODE_2D:
            kernel = clCreateKernel(g_program, "kernel2_step_float3", &error);
            break;
        default:
            abort();
    }
    if (kernel != NULL)
    {
        ocl_err(error);
        ocl_err(clFinish(g_command_queue));
        // Data buffer on GPU (RW)
        dev_bodies = clCreateBuffer(
                g_context,
                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                sizeof(Body) * n,
                bodies,
                &error);
        ocl_err(error);
        ocl_err(clFinish(g_command_queue));

        // Set kernel arguments
        cl_uint arg_num = 0;
        ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(n), &n));
        ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(dev_bodies), &dev_bodies));
        ocl_err(clFinish(g_command_queue));
    }

    CallbackData cd = {
            kernel,                 /* cl_kernel kernel */
            (size_t) n,             /* size_t n */
            bodies,                 /* Body* bodies */
            dev_bodies,             /* cl_mem dev_bodies */
            {0.7, 0.7, 1, 1},       /* Vect4f point_color */
            2,                      /* float point_size */
            true,                   /* bool draw_lines */
            {0.7, 0.7, 0.7, 0.2},   /* Vect4f line_color */
            mode,                   /* Mode mode */
    };

    renderer_start(&cd, step, draw);

    return 0;
}
