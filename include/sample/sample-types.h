/**
 * @file      sample-types.h
 * @brief     Core type definitions for the Sample framework.
 * @details   This header defines the fundamental object types and handles used throughout
 *            the Sample framework. It provides forward declarations for all major system
 *            components using opaque pointer types (#SAMPLE_TYPE).
 * @author    Author <author@email.org>
 * @copyright MIT License
 */
#ifndef SAMPLE_TYPES_H
#define SAMPLE_TYPES_H

#include "sample-platform.h"

SAMPLE_BEGIN

/**
 * @addtogroup types Types
 * @{
 */

/**
 * @name    Opaque Object Types
 * @brief   Forward declarations for framework objects using opaque pointer types
 * @details These macros generate typedefs for pointers to incomplete struct types,
 *          providing type safety while hiding implementation details. Each represents
 *          a major subsystem in the Sample framework.
 * @sa      SAMPLE_TYPE
 * @{
 */
SAMPLE_TYPE(sample_vehicle)
/** @} */

/** @} */

SAMPLE_END

#endif /* SAMPLE_TYPES_H */
