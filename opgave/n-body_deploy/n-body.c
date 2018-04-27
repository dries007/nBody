#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "time_utils.h"
#include "renderer.h"
#include "math.h"

#ifdef __APPLE__
# include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif


void usage(char* prog_name)
{
    printf("Usage: %s <number of bodies>\n", prog_name);
}

void simulate_gravity(cl_float3* host_pos, cl_float3* host_speed, int length)
{
    const float delta_time = 1.f;
    // const float grav_constant = 6.67428e-11;
    const float grav_constant = 1;
    const float mass_of_sun = 2;
    const float mass_grav = grav_constant * mass_of_sun * mass_of_sun;
    const float distance_to_nearest_star = 50;

    for (int i = 0; i < length; ++i)
    {
        for (int j = 0; j < length; ++j)
        {

            if (i == j)
                continue;

            cl_float3 pos_a = host_pos[i];
            cl_float3 pos_b = host_pos[j];

            float dist_x = (pos_a.s[0] - pos_b.s[0]) * distance_to_nearest_star;
            float dist_y = (pos_a.s[1] - pos_b.s[1]) * distance_to_nearest_star;
            float dist_z = (pos_a.s[2] - pos_b.s[2]) * distance_to_nearest_star;


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

            host_speed[i].s[0] += acc_x * delta_time;
            host_speed[i].s[1] += acc_y * delta_time;
            host_speed[i].s[2] += acc_z * delta_time;



        }
    }

    for (int i = 0; i < length; ++i)
    {
        host_pos[i].s[0] += (host_speed[i].s[0] * delta_time) / distance_to_nearest_star;
        host_pos[i].s[1] += (host_speed[i].s[1] * delta_time) / distance_to_nearest_star;
        host_pos[i].s[2] += (host_speed[i].s[2] * delta_time) / distance_to_nearest_star;
    }
}

int main(int argc, char** argv) {
    if (argc < 2)
    {
        usage(argv[0]);
        return 1;
    }
    int num_bodies = atoi(argv[1]);

    init_gl();

    cl_float3 *host_pos = malloc(sizeof(cl_float3) * num_bodies);
    cl_float3 *host_speed = malloc(sizeof(cl_float3) * num_bodies);
    for (int i = 0; i < num_bodies; ++i)
    {
        float offset;

        if (rand() < RAND_MAX / 2)
            offset = -5.f;
        else
            offset = 5.f;


        host_pos[i].s[0] = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f + offset;
        host_pos[i].s[1] = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f;
        host_pos[i].s[2] = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f;

        host_speed[i].s[0] = 0.f;
        host_speed[i].s[1] = 0.f;
        host_speed[i].s[2] = 0.f;
    }

    int is_done = 0;
    while (!is_done)
    {
        is_done = render_point_cloud(host_pos, num_bodies);
        time_measure_start("simulation step");
        simulate_gravity(host_pos, host_speed, num_bodies);
        time_measure_stop_and_print("simulation step");
    }
}
