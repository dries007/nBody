/*
 * nBody problem OpenCL kernel
 */

#ifdef FOEFELMAGIC
#define __global
#define __kernel
#define float3 struct {float x, y, z;}
#define get_global_id(dim) dim
#define sqrt(a) a
#define CLK_GLOBAL_MEM_FENCE 0
#define barrier(type) do {} while(0)
#endif

typedef union
{
    float3 vec;
    float arr[3];
} float3_;

typedef struct body
{
    float3 pos;
    float3_ speed;
} Body;

/**
 * Laptop Dries NVIDIA, 5000, avg: 0.051  s (avg 100 frames) 19.6 FPS
 * Laptop Dries NVIDIA, 1000, avg: 0.0034 s (avg 100 frames) 294  FPS
 * Laptop Dries INTEL,  5000, avg: 0.31   s (avg 100 frames)  3.2 FPS
 * Laptop Dries INTEL,  1000, avg: 0.017  s (avg 100 frames)   59 FPS
 */
__kernel void kernel_step_float3(unsigned long n, __global Body * data)
{
    const float SCALE = 50;
    const float TIME_DELTA = 1;
    const float GRAV_CONST = 1;
    const float MASS = 2;
    const float MASS_GAV = (GRAV_CONST * MASS * MASS);

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
        us->speed.vec += acc * TIME_DELTA;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);

    us->pos += us->speed.vec * (TIME_DELTA / SCALE);

    barrier(CLK_GLOBAL_MEM_FENCE);
}

/**
 * Laptop Dries NVIDIA, 5000, avg: 0.051  s (avg 100 frames) 19.6 FPS
 * Laptop Dries NVIDIA, 1000, avg: 0.0034 s (avg 100 frames)  294 FPS
 * Laptop Dries INTEL,  5000, avg: 0.30   s (avg 100 frames)  3.3 FPS
 * Laptop Dries INTEL,  1000, avg: 0.017  s (avg 100 frames)   59 FPS
 */
__kernel void kernel_step_float(unsigned long n, __global Body * data)
{
    const float SCALE = 50;
    const float TIME_DELTA = 1;
    const float GRAV_CONST = 1;
    const float MASS = 2;
    const float MASS_GAV = (GRAV_CONST * MASS * MASS);

    const int body = get_global_id(0);

    if (body >= n) return;

    __global Body* us = &data[body];

    for (int i = 0; i < n; i++)
    {
        if (i == body) continue;

        __global const Body* them = &data[i];

        float dist_x = (us->pos.x - them->pos.x) * SCALE;
        float dist_y = (us->pos.y - them->pos.y) * SCALE;
        float dist_z = (us->pos.z - them->pos.z) * SCALE;

        float dist = sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);

        float acc_x = (-MASS_GAV * dist_x / (dist * dist * dist)) / MASS;
        float acc_y = (-MASS_GAV * dist_y / (dist * dist * dist)) / MASS;
        float acc_z = (-MASS_GAV * dist_z / (dist * dist * dist)) / MASS;


        us->speed.vec.x += acc_x * TIME_DELTA;
        us->speed.vec.y += acc_y * TIME_DELTA;
        us->speed.vec.z += acc_z * TIME_DELTA;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);

    us->pos.x += us->speed.vec.x * (TIME_DELTA / SCALE);
    us->pos.y += us->speed.vec.y * (TIME_DELTA / SCALE);
    us->pos.z += us->speed.vec.z * (TIME_DELTA / SCALE);
}

// Based on: http://suhorukov.blogspot.be/2011/12/opencl-11-atomic-operations-on-floating.html
inline void AtomicAdd(volatile __global float *source, const float operand) {
    union {
        unsigned int intVal;
        float floatVal;
    } newVal;
    union {
        unsigned int intVal;
        float floatVal;
    } prevVal;
    do {
        prevVal.floatVal = *source;
        newVal.floatVal = prevVal.floatVal + operand;
    } while (atomic_cmpxchg((volatile __global unsigned int *)source, prevVal.intVal,
                            newVal.intVal) != prevVal.intVal);
}

/**
 * Laptop Dries INTEL,  1000, avg: 0.072 s (avg 100 frames)   14 FPS
 * Laptop Dries INTEL,  5000, avg: 1.5   s (avg 100 frames)  0.7 FPS
 * Laptop Dries NVIDIA, 1000, avg: 0.022 s (avg 100 frames)   45 FPS
 * Laptop Dries NVIDIA, 5000, avg: 0.47  s (avg 100 frames)    2 FPS
 */
__kernel void kernel2_step_float3(unsigned long n, __global Body * data)
{
    const float SCALE = 50;
    const float TIME_DELTA = 1;
    const float GRAV_CONST = 1;
    const float MASS = 2;
    const float MASS_GAV = (GRAV_CONST * MASS * MASS);

    const int body = get_global_id(0);

    if (body >= n) return;

    __global Body* us = &data[body];

    const int i = get_global_id(1);

    if (i >= n) return;

    if (i == body) return;

    __global const Body* them = &data[i];

    float3 dist_all = (us->pos - them->pos) * SCALE;
    float3 dist_all_sq = dist_all * dist_all;
    float dist = sqrt(dist_all_sq.x + dist_all_sq.y + dist_all_sq.z);

    float3 acc = (-MASS_GAV * dist_all / (dist * dist * dist)) / MASS;

    AtomicAdd(&us->speed.arr[0], acc.x * TIME_DELTA);
    AtomicAdd(&us->speed.arr[1], acc.y * TIME_DELTA);
    AtomicAdd(&us->speed.arr[2], acc.z * TIME_DELTA);
}