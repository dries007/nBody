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

typedef union
{
    float3 vec;
    float arr[3];
} float3_;

__kernel void add_number_to_first(__global float3_ *result)
{
    const int gid = get_global_id(0);

    float value_to_add = 5;

    AtomicAdd(&result[0].arr[0], (float)value_to_add);
    AtomicAdd(&result[0].arr[1], (float)value_to_add);
    AtomicAdd(&result[0].arr[2], (float)value_to_add);
}

