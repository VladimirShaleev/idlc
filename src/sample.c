#include "sample/sample.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NAME_LENGTH 256

struct _sample_vehicle
{
    char name[NAME_LENGTH];
    sample_vector_t velocity;
};

sample_float32_t sample_mul(sample_float32_t first, sample_float32_t second)
{
    return first * second;
}

sample_vehicle_t sample_vehicle_create(sample_utf8_t name)
{
    sample_vehicle_t instance = (sample_vehicle_t)malloc(sizeof(struct _sample_vehicle));
    memset(instance, 0, sizeof(struct _sample_vehicle));
    strncpy(instance->name, name, NAME_LENGTH);
    return instance;
}

void sample_vehicle_destroy(sample_vehicle_t vehicle)
{
    if (vehicle)
    {
        free(vehicle);
    }
}

sample_utf8_t sample_vehicle_get_name(sample_vehicle_t vehicle)
{
    assert(vehicle);
    return vehicle->name;
}

void sample_vehicle_set_velocity(sample_vehicle_t vehicle, const sample_vector_t *value)
{
    assert(vehicle);
    assert(value);
    vehicle->velocity = *value;
}

sample_float32_t sample_vehicle_dot_velocity(sample_vehicle_t vehicle, const sample_vector_t *value)
{
    assert(vehicle);
    const sample_vector_t *vec = &vehicle->velocity;
    return vec->x * value->x + vec->y * value->y + vec->z * value->z;
}
