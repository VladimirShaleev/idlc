/**
 * @file      sample.h
 * @brief     API Sample
 * @author    Author <author@email.org>
 * @copyright MIT License
 *
 */
#ifndef SAMPLE_H
#define SAMPLE_H

#include "sample-version.h"
#include "sample-types.h"

SAMPLE_BEGIN

/**
 * @brief     Function sample.
 * @param[in] first First value
 * @param[in] second Second value
 * @return    The result of multiplying *first* by *second*.
 */
sample_api sample_float32_t
sample_mul(sample_float32_t first,
           sample_float32_t second);

/**
 * @brief Vector 3.
 */
typedef struct
{
    sample_float32_t x; /**< X component */
    sample_float32_t y; /**< Y component */
    sample_float32_t z; /**< Z component */
} sample_vector_t;

/**
 * @name Functions of Vehicle.
 * @brief Functions for opaque type ::sample_vehicle_t.
 * @{
 */

/**
 * @brief     Create new vehicle instance.
 * @param[in] name Name of vehicle.
 * @return    Vehicle instance.
 */
sample_api sample_vehicle_t
sample_vehicle_create(sample_utf8_t name);

/**
 * @brief     Destroy vehicle instance.
 * @param[in] vehicle The 'this/self' object in OOP languages.
 */
sample_api void
sample_vehicle_destroy(sample_vehicle_t vehicle);

/**
 * @brief     Get name
 * @details   Get name of vehicle
 * @param[in] vehicle The 'this/self' object in OOP languages.
 * @return    Return name of vehicle
 */
sample_api sample_utf8_t
sample_vehicle_get_name(sample_vehicle_t vehicle);

/**
 * @brief     Set velocity of vehicle.
 * @param[in] vehicle The 'this/self' object in OOP languages.
 * @param[in] value Value of velocity.
 */
sample_api void
sample_vehicle_set_velocity(sample_vehicle_t vehicle,
                            const sample_vector_t* value);

/**
 * @brief     Example calculation
 * @param[in] vehicle The 'this/self' object in OOP languages.
 * @param[in] value Value of velocity.
 * @return    Dot product.
 */
sample_api sample_float32_t
sample_vehicle_dot_velocity(sample_vehicle_t vehicle,
                            const sample_vector_t* value);

/** @} */

SAMPLE_END

#endif /* SAMPLE_H */
