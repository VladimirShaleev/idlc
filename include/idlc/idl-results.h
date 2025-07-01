/**
 * @file      idl-results.h
 * @brief     Warning/Error codes.
 * @details   Here are the warning and error codes that may occur during compilation.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef IDL_RESULTS_H
#define IDL_RESULTS_H

#include "idl-version.h"
#include "idl-types.h"

IDL_BEGIN

/**
 * @brief   Code.
 * @details Enumeration of codes for results, warnings and errors.
 */
typedef enum
{
    IDL_RESULT_SUCCESS             = 0, /**< Operation completed successfully. */
    IDL_RESULT_ERROR_UNKNOWN       = 1, /**< Unknown error. */
    IDL_RESULT_ERROR_OUT_OF_MEMORY = 2, /**< Out of memory. */
    IDL_RESULT_ERROR_INVALID_ARG   = 3, /**< Invalid argument. */
    IDL_RESULT_ERROR_FILE_CREATE   = 4, /**< Failed to create file. */
    IDL_RESULT_ERROR_COMPILATION   = 5, /**< Compilation failed. */
    IDL_RESULT_ERROR_NOT_SUPPORTED = 6, /**< Not supporeted. */
    IDL_RESULT_MAX_ENUM            = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_result_t;

/**
 * @brief   TODO.
 * @details TODO.
 */
typedef enum
{
    IDL_STATUS_W1001    = 1001, /**< Missing 'author' attribute */
    IDL_STATUS_W1002    = 1002, /**< Missing 'copyright' attribute */
    IDL_STATUS_E2001    = 2001, /**< Unexpected character */
    IDL_STATUS_E2002    = 2002, /**< TODO: */
    IDL_STATUS_E2003    = 2003, /**< TODO: */
    IDL_STATUS_E2004    = 2004, /**< TODO: */
    IDL_STATUS_E2005    = 2005, /**< TODO: */
    IDL_STATUS_E2006    = 2006, /**< TODO: */
    IDL_STATUS_E2007    = 2007, /**< TODO: */
    IDL_STATUS_E2008    = 2008, /**< TODO: */
    IDL_STATUS_E2009    = 2009, /**< TODO: */
    IDL_STATUS_E2010    = 2010, /**< TODO: */
    IDL_STATUS_E2011    = 2011, /**< TODO: */
    IDL_STATUS_E2012    = 2012, /**< TODO: */
    IDL_STATUS_E2013    = 2013, /**< TODO: */
    IDL_STATUS_E2014    = 2014, /**< TODO: */
    IDL_STATUS_E2015    = 2015, /**< TODO: */
    IDL_STATUS_E2016    = 2016, /**< TODO: */
    IDL_STATUS_E2017    = 2017, /**< TODO: */
    IDL_STATUS_E2018    = 2018, /**< TODO: */
    IDL_STATUS_E2019    = 2019, /**< TODO: */
    IDL_STATUS_E2020    = 2020, /**< TODO: */
    IDL_STATUS_E2021    = 2021, /**< TODO: */
    IDL_STATUS_E2022    = 2022, /**< TODO: */
    IDL_STATUS_E2023    = 2023, /**< TODO: */
    IDL_STATUS_E2024    = 2024, /**< TODO: */
    IDL_STATUS_E2025    = 2025, /**< TODO: */
    IDL_STATUS_E2026    = 2026, /**< TODO: */
    IDL_STATUS_E2027    = 2027, /**< TODO: */
    IDL_STATUS_E2028    = 2028, /**< TODO: */
    IDL_STATUS_E2029    = 2029, /**< TODO: */
    IDL_STATUS_E2030    = 2030, /**< TODO: */
    IDL_STATUS_E2031    = 2031, /**< TODO: */
    IDL_STATUS_E2032    = 2032, /**< TODO: */
    IDL_STATUS_E2033    = 2033, /**< TODO: */
    IDL_STATUS_E2034    = 2034, /**< TODO: */
    IDL_STATUS_E2035    = 2035, /**< TODO: */
    IDL_STATUS_E2036    = 2036, /**< TODO: */
    IDL_STATUS_E2037    = 2037, /**< TODO: */
    IDL_STATUS_E2038    = 2038, /**< TODO: */
    IDL_STATUS_E2039    = 2039, /**< TODO: */
    IDL_STATUS_E2040    = 2040, /**< TODO: */
    IDL_STATUS_E2041    = 2041, /**< TODO: */
    IDL_STATUS_E2042    = 2042, /**< TODO: */
    IDL_STATUS_E2043    = 2043, /**< TODO: */
    IDL_STATUS_E2044    = 2044, /**< TODO: */
    IDL_STATUS_E2045    = 2045, /**< TODO: */
    IDL_STATUS_E2046    = 2046, /**< TODO: */
    IDL_STATUS_E2047    = 2047, /**< TODO: */
    IDL_STATUS_E2048    = 2048, /**< TODO: */
    IDL_STATUS_E2049    = 2049, /**< TODO: */
    IDL_STATUS_E2050    = 2050, /**< TODO: */
    IDL_STATUS_E2051    = 2051, /**< TODO: */
    IDL_STATUS_E2052    = 2052, /**< TODO: */
    IDL_STATUS_E2053    = 2053, /**< TODO: */
    IDL_STATUS_E2054    = 2054, /**< TODO: */
    IDL_STATUS_E2055    = 2055, /**< TODO: */
    IDL_STATUS_E2056    = 2056, /**< TODO: */
    IDL_STATUS_E2057    = 2057, /**< TODO: */
    IDL_STATUS_E2058    = 2058, /**< TODO: */
    IDL_STATUS_E2059    = 2059, /**< TODO: */
    IDL_STATUS_E2060    = 2060, /**< TODO: */
    IDL_STATUS_E2061    = 2061, /**< TODO: */
    IDL_STATUS_E2062    = 2062, /**< TODO: */
    IDL_STATUS_E2063    = 2063, /**< TODO: */
    IDL_STATUS_E2064    = 2064, /**< TODO: */
    IDL_STATUS_E2065    = 2065, /**< TODO: */
    IDL_STATUS_E2066    = 2066, /**< TODO: */
    IDL_STATUS_E2067    = 2067, /**< TODO: */
    IDL_STATUS_E2068    = 2068, /**< TODO: */
    IDL_STATUS_E2069    = 2069, /**< TODO: */
    IDL_STATUS_E2070    = 2070, /**< TODO: */
    IDL_STATUS_E2071    = 2071, /**< TODO: */
    IDL_STATUS_E2072    = 2072, /**< TODO: */
    IDL_STATUS_E2073    = 2073, /**< TODO: */
    IDL_STATUS_E2074    = 2074, /**< TODO: */
    IDL_STATUS_E2075    = 2075, /**< TODO: */
    IDL_STATUS_E2076    = 2076, /**< TODO: */
    IDL_STATUS_E2077    = 2077, /**< TODO: */
    IDL_STATUS_E2078    = 2078, /**< TODO: */
    IDL_STATUS_E2079    = 2079, /**< TODO: */
    IDL_STATUS_E2080    = 2080, /**< TODO: */
    IDL_STATUS_E2081    = 2081, /**< TODO: */
    IDL_STATUS_E2082    = 2082, /**< TODO: */
    IDL_STATUS_E2083    = 2083, /**< TODO: */
    IDL_STATUS_E2084    = 2084, /**< TODO: */
    IDL_STATUS_E2085    = 2085, /**< TODO: */
    IDL_STATUS_E2086    = 2086, /**< TODO: */
    IDL_STATUS_E2087    = 2087, /**< TODO: */
    IDL_STATUS_E2088    = 2088, /**< TODO: */
    IDL_STATUS_E2089    = 2089, /**< TODO: */
    IDL_STATUS_E2090    = 2090, /**< TODO: */
    IDL_STATUS_E2091    = 2091, /**< TODO: */
    IDL_STATUS_E2092    = 2092, /**< TODO: */
    IDL_STATUS_E2093    = 2093, /**< TODO: */
    IDL_STATUS_E2094    = 2094, /**< TODO: */
    IDL_STATUS_E2095    = 2095, /**< TODO: */
    IDL_STATUS_E2096    = 2096, /**< TODO: */
    IDL_STATUS_E2097    = 2097, /**< TODO: */
    IDL_STATUS_E2098    = 2098, /**< TODO: */
    IDL_STATUS_E2099    = 2099, /**< TODO: */
    IDL_STATUS_E2100    = 2100, /**< TODO: */
    IDL_STATUS_E2101    = 2101, /**< TODO: */
    IDL_STATUS_E2102    = 2102, /**< TODO: */
    IDL_STATUS_E2103    = 2103, /**< TODO: */
    IDL_STATUS_E2104    = 2104, /**< TODO: */
    IDL_STATUS_E2105    = 2105, /**< TODO: */
    IDL_STATUS_E2106    = 2106, /**< TODO: */
    IDL_STATUS_E2107    = 2107, /**< TODO: */
    IDL_STATUS_E2108    = 2108, /**< TODO: */
    IDL_STATUS_E2109    = 2109, /**< TODO: */
    IDL_STATUS_E2110    = 2110, /**< TODO: */
    IDL_STATUS_E2111    = 2111, /**< TODO: */
    IDL_STATUS_E2112    = 2112, /**< TODO: */
    IDL_STATUS_E2113    = 2113, /**< TODO: */
    IDL_STATUS_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_status_t;

/**
 * @brief   TODO.
 * @details TODO.
 */
typedef struct
{
    idl_status_t status; /**< TODO. */
    idl_utf8_t   message; /**< TODO. */
    idl_utf8_t   filename; /**< TODO. */
    idl_uint32_t line; /**< TODO. */
    idl_uint32_t column; /**< TODO. */
} idl_message_t;

/**
 * @brief     Converts error code to descriptive string.
 * @details   Provides error message for debugging.
 * @param[in] result Error code value.
 * @return    Corresponding error message.
 */
idl_api idl_utf8_t
idl_result_to_string(idl_result_t result);

/**
 * @brief     Increments reference count.
 * @details   Manages compilation result instance lifetime.
 * @param[in] compilation_result Target compilation result instance.
 * @return    Reference to same compilation result.
 * @sa        ::idl_compilation_result_destroy
 */
idl_api idl_compilation_result_t
idl_compilation_result_reference(idl_compilation_result_t compilation_result);

/**
 * @brief     Releases compilation result instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] compilation_result Compilation result to destroy.
 * @sa        ::idl_compilation_result_reference
 */
idl_api void
idl_compilation_result_destroy(idl_compilation_result_t compilation_result);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] compilation_result TODO.
 * @return    TODO.
 */
idl_api idl_bool_t
idl_compilation_result_has_warnings(idl_compilation_result_t compilation_result);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] compilation_result TODO.
 * @return    TODO.
 */
idl_api idl_bool_t
idl_compilation_result_has_errors(idl_compilation_result_t compilation_result);

/**
 * @brief         TODO.
 * @details       TODO.
 * @param[in]     compilation_result TODO.
 * @param[in,out] message_count TODO.
 * @param[out]    messages TODO.
 */
idl_api void
idl_compilation_result_get_messages(idl_compilation_result_t compilation_result,
                                    idl_uint32_t* message_count,
                                    idl_message_t* messages);

IDL_END

#endif /* IDL_RESULTS_H */
