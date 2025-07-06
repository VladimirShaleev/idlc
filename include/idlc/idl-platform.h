/**
 * @file      idl-platform.h
 * @brief     Platform-specific definitions and utilities.
 * @details   This header provides cross-platform macros, type definitions, and utility
 *            macros for the Idl library. It handles:
 *            - Platform detection (Windows, macOS, iOS, Android, Linux, Web)
 *            - Symbol visibility control (DLL import/export on Windows)
 *            - C/C++ interoperability
 *            - Type definitions for consistent data sizes across platforms
 *            - Bit flag operations for enumerations (C++ only).
 *            
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @ingroup   files
 * @copyright MIT License
 */
#ifndef IDL_PLATFORM_H
#define IDL_PLATFORM_H

/**
 * @def     IDL_BEGIN
 * @brief   Begins a C-linkage declaration block.
 * @details In C++, expands to `extern "C" {` to ensure C-compatible symbol naming.
 *          In pure C environments, expands to nothing.
 * @sa      IDL_END
 * @ingroup macros
 *
 */

/**
 * @def     IDL_END
 * @brief   Ends a C-linkage declaration block.
 * @details Closes the scope opened by #IDL_BEGIN.
 * @sa      IDL_BEGIN
 * @ingroup macros
 *
 */

#ifdef __cplusplus
# define IDL_BEGIN extern "C" {
# define IDL_END   }
#else
# define IDL_BEGIN
# define IDL_END
#endif

/**
 * @def     idl_api
 * @brief   Controls symbol visibility for shared library builds.
 * @details This macro is used to control symbol visibility when building or using the library.
 *          On Windows (**MSVC**) with dynamic linking (non-static build), it expands to `__declspec(dllimport)`.
 *          In all other cases (static builds or non-Windows platforms), it expands to nothing.
 *          This allows proper importing of symbols from DLLs on Windows platforms.
 * @note    Define `IDL_STATIC_BUILD` for static library configuration.
 * @ingroup macros
 */

#ifndef idl_api
# if defined(_MSC_VER) && !defined(IDL_STATIC_BUILD)
#  define idl_api __declspec(dllimport)
# else
#  define idl_api
# endif
#endif

#if defined(_WIN32) && !defined(IDL_PLATFORM_WINDOWS)
# define IDL_PLATFORM_WINDOWS
#elif defined(__APPLE__)
# include <TargetConditionals.h>
# include <unistd.h>
# if TARGET_OS_IPHONE && !defined(IDL_PLATFORM_IOS)
#  define IDL_PLATFORM_IOS
# elif TARGET_IPHONE_SIMULATOR && !defined(IDL_PLATFORM_IOS)
#  define IDL_PLATFORM_IOS
# elif TARGET_OS_MAC && !defined(IDL_PLATFORM_MAC_OS)
#  define IDL_PLATFORM_MAC_OS
# else
#  error unsupported Apple platform
# endif
#elif defined(__ANDROID__) && !defined(IDL_PLATFORM_ANDROID)
# define IDL_PLATFORM_ANDROID
#elif defined(__linux__) && !defined(IDL_PLATFORM_LINUX)
# define IDL_PLATFORM_LINUX
#elif defined(__EMSCRIPTEN__) && !defined(IDL_PLATFORM_WEB)
# define IDL_PLATFORM_WEB
#else
# error unsupported platform
#endif

#ifdef __cpp_constexpr
#  define IDL_CONSTEXPR constexpr
#  if __cpp_constexpr >= 201304L
#    define IDL_CONSTEXPR_14 constexpr
#  else
#    define IDL_CONSTEXPR_14
#  endif
#else
#  define IDL_CONSTEXPR
#  define IDL_CONSTEXPR_14
#endif

/**
 * @addtogroup types Types
 * @{
 */

/**
 * @name  Platform-independent type definitions.
 * @brief Fixed-size types guaranteed to work across all supported platforms.
 * @{
 */
#include <stdint.h>
typedef char        idl_char_t;    /**< symbol type. */
typedef int32_t     idl_bool_t;    /**< boolean type. */
typedef int8_t      idl_sint8_t;   /**< 8 bit signed integer. */
typedef uint8_t     idl_uint8_t;   /**< 8 bit unsigned integer. */
typedef int16_t     idl_sint16_t;  /**< 16 bit signed integer. */
typedef uint16_t    idl_uint16_t;  /**< 16 bit unsigned integer. */
typedef int32_t     idl_sint32_t;  /**< 32 bit signed integer. */
typedef uint32_t    idl_uint32_t;  /**< 32 bit unsigned integer. */
typedef int64_t     idl_sint64_t;  /**< 64 bit signed integer. */
typedef uint64_t    idl_uint64_t;  /**< 64 bit unsigned integer. */
typedef float       idl_float32_t; /**< 32 bit float point. */
typedef double      idl_float64_t; /**< 64 bit float point. */
typedef const char* idl_utf8_t;    /**< utf8 string. */
typedef void*       idl_data_t;    /**< pointer to data. */
typedef const void* idl_cdata_t;   /**< pointer to immutable data. */
/** @} */

/** @} */

/**
 * @def       IDL_FLAGS
 * @brief     Enables bit flag operations for enumerations (C++ only).
 * @details   Generates overloaded bitwise operators for type-safe flag manipulation:
 *            - Bitwise NOT (~)
 *            - OR (|, |=)
 *            - AND (&, &=)
 *            - XOR (^, ^=)
 * 
 * @param[in] idl_enum_t Enumeration type to enhance with flag operations.
 * @note      Only active in C++ mode. In C, expands to nothing.
 * @ingroup   macros
 */

#ifdef __cplusplus
# define IDL_FLAGS(idl_enum_t) \
extern "C++" { \
inline IDL_CONSTEXPR idl_enum_t operator~(idl_enum_t lhr) noexcept { \
    return static_cast<idl_enum_t>(~static_cast<idl_sint32_t>(lhr)); \
} \
inline IDL_CONSTEXPR idl_enum_t operator|(idl_enum_t lhr, idl_enum_t rhs) noexcept { \
    return static_cast<idl_enum_t>(static_cast<idl_sint32_t>(lhr) | static_cast<idl_sint32_t>(rhs)); \
} \
inline IDL_CONSTEXPR idl_enum_t operator&(idl_enum_t lhr, idl_enum_t rhs) noexcept { \
    return static_cast<idl_enum_t>(static_cast<idl_sint32_t>(lhr) & static_cast<idl_sint32_t>(rhs)); \
} \
inline IDL_CONSTEXPR idl_enum_t operator^(idl_enum_t lhr, idl_enum_t rhs) noexcept { \
    return static_cast<idl_enum_t>(static_cast<idl_sint32_t>(lhr) ^ static_cast<idl_sint32_t>(rhs)); \
} \
inline IDL_CONSTEXPR_14 idl_enum_t& operator|=(idl_enum_t& lhr, idl_enum_t rhs) noexcept { \
    return lhr = lhr | rhs; \
} \
inline IDL_CONSTEXPR_14 idl_enum_t& operator&=(idl_enum_t& lhr, idl_enum_t rhs) noexcept { \
    return lhr = lhr & rhs; \
} \
inline IDL_CONSTEXPR_14 idl_enum_t& operator^=(idl_enum_t& lhr, idl_enum_t rhs) noexcept { \
    return lhr = lhr ^ rhs; \
} \
}
#else
# define IDL_FLAGS(idl_enum_t)
#endif

/**
 * @def       IDL_TYPE
 * @brief     Declares an opaque handle type.
 * @details   Creates a typedef for a pointer to an incomplete struct type,
 *            providing type safety while hiding implementation details.
 * @param[in] idl_name Base name for the type (suffix `_t` will be added).
 * @ingroup   macros
 */
#define IDL_TYPE(idl_name) \
typedef struct _##idl_name* idl_name##_t;

#endif /* IDL_PLATFORM_H */
