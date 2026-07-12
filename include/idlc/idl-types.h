/**
 * @file      idl-types.h
 * @brief     Core type definitions for the Idl framework.
 * @details   This header defines the fundamental object types and handles used throughout
 *            the Idl framework. It provides forward declarations for all major system
 *            components using opaque pointer types (#IDL_TYPE) and index-based handles
 *            (#IDL_HANDLE) for better type safety and abstraction.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @ingroup   files
 * @copyright MIT License
 */
#ifndef IDL_TYPES_H
#define IDL_TYPES_H

#include "idl-platform.h"

IDL_BEGIN

/**
 * @addtogroup types Types
 * @{
 */

/**
 * @name    Opaque Object Types
 * @brief   Forward declarations for framework objects using opaque pointer types
 * @details These macros generate typedefs for pointers to incomplete struct types,
 *          providing type safety while hiding implementation details. Each represents
 *          a major subsystem in the Idl framework.
 * @sa      IDL_TYPE
 * @{
 */
IDL_TYPE(idl_compilation_result) /**< Compilation result. */
IDL_TYPE(idl_options)            /**< Compilation options. */
IDL_TYPE(idl_compiler)           /**< Compiler interface. */
/** @} */

/**
 * @name    Resource Handles
 * @brief   Index-based handles
 * @details These macros generate lightweight handle types,
 *          using indices rather than pointers for better memory management
 *          and cross-API compatibility. Each handle contains an internal index.
 * @sa      IDL_HANDLE
 * @{
 */
IDL_HANDLE(idl_ast_node) /**< TODO. */
/** @} */

/** @} */

IDL_END

#endif /* IDL_TYPES_H */
