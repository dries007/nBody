/**
 * (c) Dries Kennes & Stijn Van Dessel 2018
 *
 * nBody problem main
 */
#include <stdio.h>
#include <stdlib.h>

#include "lib/ocl_utils.h"
#include "lib/time_utils.h"

typedef struct __attribute__((packed)) body
{
    cl_float3 pos;
    cl_float3 speed;
} Body;

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
    }

    return data;
}

int main(int argc, char ** argv)
{
    if (argc < 1)
    {
        printf("%s <number of bodies>\n", argv[0]);
        return -1;
    }
    int n = atoi(argv[1]);
    if (n < 1)
    {
        printf("%d < 1\n", n);
        return -1;
    }

    cl_platform_id platform = ocl_select_platform();
    cl_device_id device = ocl_select_device(platform);
    init_ocl(device);
    create_program("kernel.cl", "");

    time_measure_start("init");

    Body* bodies = create_bodies((size_t) n);

    cl_int error;


    // Data buffer on GPU (RW)
    cl_mem dev_bodies = clCreateBuffer(g_context, CL_MEM_READ_WRITE, sizeof(Body) * n, NULL, &error);
    ocl_err(error);
    ocl_err(clFinish(g_command_queue));

    // Create kernel
    cl_kernel kernel = clCreateKernel(g_program, "kernel_step", &error);
    ocl_err(error);
    ocl_err(clFinish(g_command_queue));

    // Set kernel arguments
    cl_uint arg_num = 0;
    ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(cl_uint), &n));
    ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(cl_uint), &dev_bodies));
    ocl_err(clFinish(g_command_queue));

    size_t global_work_sizes[] = {(size_t) n};
    time_measure_stop_and_print("init");


    time_measure_start("runtime");
    for (int i = 0; i < 1000; i++);
    {
        // Call kernel
//        time_measure_start("computation");
        ocl_err(clEnqueueNDRangeKernel(g_command_queue, kernel, 1, NULL, global_work_sizes, NULL, 0, NULL, NULL));
        ocl_err(clFinish(g_command_queue));
//        time_measure_stop_and_print("computation");

        // Save results
//        time_measure_start("copy");
        ocl_err(clEnqueueReadBuffer(g_command_queue, dev_bodies, CL_TRUE, 0, sizeof(Body) * n, bodies, 0, NULL, NULL));
        ocl_err(clFinish(g_command_queue));
//        time_measure_stop_and_print("copy");

        // todo: add showing of data.
    }
    time_measure_stop_and_print("runtime");

    return 0;
}
