/**
 * (c) Dries Kennes & Stijn Van Dessel 2018
 *
 * nBody problem
 */
#include "lib/ocl_utils.h"
#include "lib/time_utils.h"
#include "lib/renderer.h"

typedef struct __attribute__((packed)) body
{
    cl_float3 pos;
    cl_float3 speed;
} Body;

typedef struct callback_data {
    cl_kernel kernel;
    size_t n;
    Body* bodies;
    cl_mem dev_bodies;
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

    glColor4f(1, 1, 1, 1);
    glPointSize(3);
    glBegin(GL_POINTS);
    for (int i = 0; i < cd->n; i++)
    {
//        printf("%d:%.3f;%.3f;%.3f ", i, cd->bodies[i].pos.x, cd->bodies[i].pos.y, cd->bodies[i].pos.z);
        glVertex3fv((float *) &cd->bodies[i].pos);
    }
//    printf("\n");
    glEnd();

    glColor4f(.5, .5, .5, .3);
    glBegin(GL_LINES);
    for (int i = 0; i < cd->n; i++)
    {
        glVertex3fv((float *) &cd->bodies[i].pos);
        glVertex3f(cd->bodies[i].pos.x + cd->bodies[i].speed.x, cd->bodies[i].pos.y + cd->bodies[i].speed.y, cd->bodies[i].pos.z + cd->bodies[i].speed.z);
    }
    glEnd();
}

void step(void* data)
{
//    time_measure_start("step");
    CallbackData* cd = data;
    ocl_err(clEnqueueNDRangeKernel(g_command_queue, cd->kernel, 1, NULL, &cd->n, NULL, 0, NULL, NULL));
    ocl_err(clFinish(g_command_queue));
    ocl_err(clEnqueueReadBuffer(g_command_queue, cd->dev_bodies, CL_TRUE, 0, sizeof(Body) * cd->n, cd->bodies, 0, NULL, NULL));
    ocl_err(clFinish(g_command_queue));
//    time_measure_stop_and_print("step");
}

int main(int argc, char ** argv)
{
    if (argc <= 1)
    {
        printf("%s <number of bodies>\n", argv[0]);
        return -1;
    }
    errno = 0;
    cl_ulong n = strtoul(argv[1], NULL, 0);
    if (errno != 0)
    {
        printf("'%s' was not converted to a proper number. (%ld)\n", argv[1], n);
        return -1;
    }

    cl_platform_id platform = ocl_select_platform();
    cl_device_id device = ocl_select_device(platform);
    init_ocl(device);
    create_program("kernel.cl", "");

    char* title = malloc(1 + (size_t) snprintf(NULL, 0, "nBody problem - %ld bodies", n));
    sprintf(title, "nBody problem - %ld bodies", n);
    renderer_init(argc, argv, title);

    srand((unsigned int) time(NULL));
    Body* bodies = create_bodies((size_t) n);

    cl_int error;

    // Data buffer on GPU (RW)
    cl_mem dev_bodies = clCreateBuffer(g_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Body) * n, bodies, &error);
    ocl_err(error);
    ocl_err(clFinish(g_command_queue));

    // Create kernel
    cl_kernel kernel = clCreateKernel(g_program, "kernel_step", &error);
    ocl_err(error);
    ocl_err(clFinish(g_command_queue));
    // Set kernel arguments
    cl_uint arg_num = 0;
    ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(n), &n));
    ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(dev_bodies), &dev_bodies));
    ocl_err(clFinish(g_command_queue));

    CallbackData cd = {
            kernel,
            (size_t) n,
            bodies,
            dev_bodies,
    };

    renderer_start(&cd, step, draw);

    return 0;
}
