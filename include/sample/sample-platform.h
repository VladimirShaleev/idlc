/**
 * @file      sample-platform.h
 * @brief     Platform-specific definitions and utilities.
 * @details   This header provides cross-platform macros, type definitions, and utility
 *            macros for the Sample library. It handles:
 *            - Platform detection (Windows, macOS, iOS, Android, Linux, Web)
 *            - Symbol visibility control (DLL import/export on Windows)
 *            - C/C++ interoperability
 *            - Type definitions for consistent data sizes across platforms
 *            - Bit flag operations for enumerations (C++ only).
 *            
 * @author    Author <author@email.org>
 * @copyright MIT License
 */
#ifndef SAMPLE_PLATFORM_H
#define SAMPLE_PLATFORM_H

/**
 * @def     SAMPLE_BEGIN
 * @brief   Begins a C-linkage declaration block.
 * @details In C++, expands to `extern "C" {` to ensure C-compatible symbol naming.
 *          In pure C environments, expands to nothing.
 * @sa      SAMPLE_END
 *
 */

/**
 * @def     SAMPLE_END
 * @brief   Ends a C-linkage declaration block.
 * @details Closes the scope opened by #SAMPLE_BEGIN.
 * @sa      SAMPLE_BEGIN
 *
 */

#ifdef __cplusplus
# define SAMPLE_BEGIN extern "C" {
# define SAMPLE_END   }
#else
# define SAMPLE_BEGIN
# define SAMPLE_END
#endif

/**
 * @def     sample_api
 * @brief   Controls symbol visibility for shared library builds.
 * @details This macro is used to control symbol visibility when building or using the library.
 *          On Windows (**MSVC**) with dynamic linking (non-static build), it expands to `__declspec(dllimport)`.
 *          In all other cases (static builds or non-Windows platforms), it expands to nothing.
 *          This allows proper importing of symbols from DLLs on Windows platforms.
 * @note    Define `SAMPLE_STATIC_BUILD` for static library configuration.
 */

#ifndef sample_api
# if defined(_MSC_VER) && !defined(SAMPLE_STATIC_BUILD)
#  define sample_api __declspec(dllimport)
# else
#  define sample_api
# endif
#endif

#if defined(_WIN32) && !defined(SAMPLE_PLATFORM_WINDOWS)
# define SAMPLE_PLATFORM_WINDOWS
#elif defined(__APPLE__)
# include <TargetConditionals.h>
# include <unistd.h>
# if TARGET_OS_IPHONE && !defined(SAMPLE_PLATFORM_IOS)
#  define SAMPLE_PLATFORM_IOS
# elif TARGET_IPHONE_SIMULATOR && !defined(SAMPLE_PLATFORM_IOS)
#  define SAMPLE_PLATFORM_IOS
# elif TARGET_OS_MAC && !defined(SAMPLE_PLATFORM_MAC_OS)
#  define SAMPLE_PLATFORM_MAC_OS
# else
#  error unsupported Apple platform
# endif
#elif defined(__ANDROID__) && !defined(SAMPLE_PLATFORM_ANDROID)
# define SAMPLE_PLATFORM_ANDROID
#elif defined(__linux__) && !defined(SAMPLE_PLATFORM_LINUX)
# define SAMPLE_PLATFORM_LINUX
#elif defined(__EMSCRIPTEN__) && !defined(SAMPLE_PLATFORM_WEB)
# define SAMPLE_PLATFORM_WEB
#else
# error unsupported platform
#endif

#ifdef __cpp_constexpr
#  define SAMPLE_CONSTEXPR constexpr
#  if __cpp_constexpr >= 201304L
#    define SAMPLE_CONSTEXPR_14 constexpr
#  else
#    define SAMPLE_CONSTEXPR_14
#  endif
#else
#  define SAMPLE_CONSTEXPR
#  define SAMPLE_CONSTEXPR_14
#endif

/**
 * @name  Platform-independent type definitions.
 * @brief Fixed-size types guaranteed to work across all supported platforms.
 * @{
 */
#include <stdint.h>
typedef char        sample_char_t;    /**< symbol type. */
typedef int32_t     sample_bool_t;    /**< boolean type. */
typedef int8_t      sample_sint8_t;   /**< 8 bit signed integer. */
typedef uint8_t     sample_uint8_t;   /**< 8 bit unsigned integer. */
typedef int16_t     sample_sint16_t;  /**< 16 bit signed integer. */
typedef uint16_t    sample_uint16_t;  /**< 16 bit unsigned integer. */
typedef int32_t     sample_sint32_t;  /**< 32 bit signed integer. */
typedef uint32_t    sample_uint32_t;  /**< 32 bit unsigned integer. */
typedef int64_t     sample_sint64_t;  /**< 64 bit signed integer. */
typedef uint64_t    sample_uint64_t;  /**< 64 bit unsigned integer. */
typedef float       sample_float32_t; /**< 32 bit float point. */
typedef double      sample_float64_t; /**< 64 bit float point. */
typedef const char* sample_utf8_t;    /**< utf8 string. */
typedef void*       sample_data_t;    /**< pointer to data. */
typedef const void* sample_cdata_t;   /**< pointer to immutable data. */
/** @} */

/**
 * @def       SAMPLE_FLAGS
 * @brief     Enables bit flag operations for enumerations (C++ only).
 * @details   Generates overloaded bitwise operators for type-safe flag manipulation:
 *            - Bitwise NOT (~)
 *            - OR (|, |=)
 *            - AND (&, &=)
 *            - XOR (^, ^=)
 * 
 * @param[in] sample_enum_t Enumeration type to enhance with flag operations.
 * @note      Only active in C++ mode. In C, expands to nothing.
 */

#ifdef __cplusplus
# define SAMPLE_FLAGS(sample_enum_t) \
extern "C++" { \
inline SAMPLE_CONSTEXPR sample_enum_t operator~(sample_enum_t lhr) noexcept { \
    return static_cast<sample_enum_t>(~static_cast<sample_sint32_t>(lhr)); \
} \
inline SAMPLE_CONSTEXPR sample_enum_t operator|(sample_enum_t lhr, sample_enum_t rhs) noexcept { \
    return static_cast<sample_enum_t>(static_cast<sample_sint32_t>(lhr) | static_cast<sample_sint32_t>(rhs)); \
} \
inline SAMPLE_CONSTEXPR sample_enum_t operator&(sample_enum_t lhr, sample_enum_t rhs) noexcept { \
    return static_cast<sample_enum_t>(static_cast<sample_sint32_t>(lhr) & static_cast<sample_sint32_t>(rhs)); \
} \
inline SAMPLE_CONSTEXPR sample_enum_t operator^(sample_enum_t lhr, sample_enum_t rhs) noexcept { \
    return static_cast<sample_enum_t>(static_cast<sample_sint32_t>(lhr) ^ static_cast<sample_sint32_t>(rhs)); \
} \
inline SAMPLE_CONSTEXPR_14 sample_enum_t& operator|=(sample_enum_t& lhr, sample_enum_t rhs) noexcept { \
    return lhr = lhr | rhs; \
} \
inline SAMPLE_CONSTEXPR_14 sample_enum_t& operator&=(sample_enum_t& lhr, sample_enum_t rhs) noexcept { \
    return lhr = lhr & rhs; \
} \
inline SAMPLE_CONSTEXPR_14 sample_enum_t& operator^=(sample_enum_t& lhr, sample_enum_t rhs) noexcept { \
    return lhr = lhr ^ rhs; \
} \
}
#else
# define SAMPLE_FLAGS(sample_enum_t)
#endif

/**
 * @def       SAMPLE_TYPE
 * @brief     Declares an opaque handle type.
 * @details   Creates a typedef for a pointer to an incomplete struct type,
 *            providing type safety while hiding implementation details.
 * @param[in] sample_name Base name for the type (suffix `_t` will be added).
 */
#define SAMPLE_TYPE(sample_name) \
typedef struct _##sample_name* sample_name##_t;

#endif /* SAMPLE_PLATFORM_H */
