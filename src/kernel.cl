/*
 * nBody problem OpenCL kernel
 */
typedef struct body
{
    float3 pos;
    float3 speed;
} Body;

#ifdef FOEFELMAGIC
#define __global
#define __kernel
#endif

__kernel void kernel_step(unsigned long n, __global Body * data)
{
    const float SCALE = 50;
    const float TIME_DELTA = .1;
    const float GRAV_CONST = 1;
    const float MASS = 2;
    const float MASS_GAV = GRAV_CONST * MASS * MASS;

    const int body = get_global_id(0);

    if (body >= n) return;

    __global Body* us = &data[body];

    for (int i = 0; i < n; i++)
    {
        if (i == body) continue;

        __global const Body* them = &data[i];

        float3 dist_all = (us->pos - them->pos) * SCALE;
        float3 dist_all_sq = dist_all * dist_all;
        float dist = sqrt(dist_all_sq.x + dist_all_sq.y + dist_all_sq.z);

        float3 acc = (-MASS_GAV * dist_all / (dist * dist * dist)) / MASS;
        us->speed += acc * TIME_DELTA;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);

    us->pos += us->speed * (TIME_DELTA / SCALE);

    barrier(CLK_GLOBAL_MEM_FENCE);
}
