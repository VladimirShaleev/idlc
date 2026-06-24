/**
 * @file      idl-results.h
 * @brief     Warning/Error codes.
 * @details   Here are the warning and error codes that may occur during compilation.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @ingroup   files
 * @copyright MIT License
 */
#ifndef IDL_RESULTS_H
#define IDL_RESULTS_H

#include "idl-version.h"
#include "idl-types.h"

IDL_BEGIN

/**
 * @brief   Result codes.
 * @details Enumeration of result codes.
 * @ingroup enums
 */
typedef enum
{
    IDL_RESULT_SUCCESS             = 0, /**< Indicates success (this is not an error). */
    IDL_RESULT_ERROR_UNKNOWN       = 1, /**< Unknown error. */
    IDL_RESULT_ERROR_OUT_OF_MEMORY = 2, /**< Out of memory. */
    IDL_RESULT_ERROR_INVALID_ARG   = 3, /**< Invalid argument. */
    IDL_RESULT_ERROR_FILE_CREATE   = 4, /**< Failed to create file. */
    IDL_RESULT_ERROR_COMPILATION   = 5, /**< Compilation failed. */
    IDL_RESULT_ERROR_NOT_SUPPORTED = 6, /**< Not supporeted. */
    IDL_RESULT_MAX_ENUM            = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_result_t;

/**
 * @brief   Compilation statuses.
 * @details This enumeration contains warnings and errors that may occur during compilation.
 * @ingroup enums
 */
typedef enum
{
    IDL_STATUS_N1001    = 1001, /**< Unnecessary parentheses for a parameterless attribute. */
    IDL_STATUS_W2001    = 2001, /**< The declaration is missing an attribute. */
    IDL_STATUS_W2002    = 2002, /**< Repeated import. */
    IDL_STATUS_E3001    = 3001, /**< Syntax error. */
    IDL_STATUS_E3002    = 3002, /**< Argument parsing error. */
    IDL_STATUS_E3003    = 3003, /**< The [version] attribute must have three required integer parameters, such as version(1, 2, 3) or version("string"). */
    IDL_STATUS_E3004    = 3004, /**< Version values must be between 0 and 255. */
    IDL_STATUS_E3005    = 3005, /**< An invalid attribute was specified for the declaration. */
    IDL_STATUS_E3006    = 3006, /**< Attributes are not allowed for the declaration. */
    IDL_STATUS_E3007    = 3007, /**< Attribute duplication. */
    IDL_STATUS_E3008    = 3008, /**< The attribute must not have arguments. */
    IDL_STATUS_E3009    = 3009, /**< String closing character not found. */
    IDL_STATUS_E3010    = 3010, /**< API Redeclaration. */
    IDL_STATUS_E3011    = 3011, /**< The first declaration in the description should always begin with the 'api' declaration. */
    IDL_STATUS_E3012    = 3012, /**< Symbol redefinition. */
    IDL_STATUS_E3013    = 3013, /**< Unknown attribute. */
    IDL_STATUS_E3014    = 3014, /**< The [brief] attribute must contain one or more arguments. */
    IDL_STATUS_E3015    = 3015, /**< Unknown attribute in the documentation. */
    IDL_STATUS_E3016    = 3016, /**< The documentation string cannot be empty. */
    IDL_STATUS_E3017    = 3017, /**< The [detail] attribute must contain one or more arguments. */
    IDL_STATUS_E3018    = 3018, /**< Inline documentation only [detail] description is allowed. */
    IDL_STATUS_E3019    = 3019, /**< The [order] attribute can contain one optional Boolean parameter. */
    IDL_STATUS_E3020    = 3020, /**< Tabs are not allowed. */
    IDL_STATUS_E3021    = 3021, /**< Could not find file for import. */
    IDL_STATUS_E3022    = 3022, /**< Failed to open file. */
    IDL_STATUS_E3023    = 3023, /**< A 'const' can be defined only for an 'enum'. */
    IDL_STATUS_E3024    = 3024, /**< The [value] attribute must contain one or more arguments. */
    IDL_STATUS_E3025    = 3025, /**< Arguments for the [value] attribute must be literals or declaration reference. */
    IDL_STATUS_E3026    = 3026, /**< All literals in the [value] attribute must be of the same type. */
    IDL_STATUS_E3027    = 3027, /**< The [type] attribute argument can only refer to symbols. */
    IDL_STATUS_E3028    = 3028, /**< The [cname] attribute must contain a single string literal argument. */
    IDL_STATUS_E3029    = 3029, /**< The [cname] attribute must specify a name without spaces and punctuations. */
    IDL_STATUS_E3030    = 3030, /**< The [single] attribute can contain one optional Boolean parameter. */
    IDL_STATUS_E3031    = 3031, /**< Invalid tokenizer format string, a valid string looks like (2-^3-4). */
    IDL_STATUS_E3032    = 3032, /**< Integer tokenization parameters or a tokenizer string must be passed to the attribute [tokenizer]. */
    IDL_STATUS_E3033    = 3033, /**< The [tokeinzer] attribute must contain one or more arguments (integers: 2, -2, 4 or string "2-^3-4"). */
    IDL_STATUS_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_status_t;

/**
 * @brief   Compilation message.
 * @details Detailed description of warning or compilation error.
 * @ingroup structs
 */
typedef struct
{
    idl_status_t status; /**< Compilation status. */
    idl_bool_t   is_error; /**< The message indicates an error. */
    idl_utf8_t   message; /**< Detailed text description. */
    idl_utf8_t   filename; /**< File in which warning or error was detected. */
    idl_uint32_t line; /**< The line number where the warning or error was detected. */
    idl_uint32_t column; /**< The column in which the warning or error was detected. */
} idl_message_t;

/**
 * @brief     Converts error code to descriptive string.
 * @details   Provides a text description for the result code.
 * @param[in] result Result code.
 * @return    Corresponding text description of the result code.
 * @ingroup   functions
 */
idl_api idl_utf8_t
idl_result_to_string(idl_result_t result);

/**
 * @name Functions of Compilation Result.
 * @brief Functions for opaque type ::idl_compilation_result_t.
 * @{
 */

/**
 * @brief     Increments reference count.
 * @details   Manages compilation result instance lifetime.
 * @param[in] compilation_result Target compilation result instance.
 * @return    Reference to same compilation result.
 * @sa        ::idl_compilation_result_destroy
 * @ingroup   functions
 */
idl_api idl_compilation_result_t
idl_compilation_result_reference(idl_compilation_result_t compilation_result);

/**
 * @brief     Releases compilation result instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] compilation_result Compilation result to destroy.
 * @sa        ::idl_compilation_result_reference
 * @ingroup   functions
 */
idl_api void
idl_compilation_result_destroy(idl_compilation_result_t compilation_result);

/**
 * @brief     Checking if there were warnings.
 * @details   Check if there were any warnings during compilation.
 * @param[in] compilation_result Target compilation result instance.
 * @return    *TRUE*, if there are warnings.
 * @ingroup   functions
 */
idl_api idl_bool_t
idl_compilation_result_has_warnings(idl_compilation_result_t compilation_result);

/**
 * @brief     Checking if there were errors.
 * @details   Check if there were any errors during compilation.
 * @param[in] compilation_result Target compilation result instance.
 * @return    *TRUE*, if there are errors.
 * @ingroup   functions
 */
idl_api idl_bool_t
idl_compilation_result_has_errors(idl_compilation_result_t compilation_result);

/**
 * @brief         Returns messages with warnings and errors.
 * @details       Returns messages with warnings and errors that occurred during compilation.
 * @param[in]     compilation_result Target compilation result instance.
 * @param[in,out] message_count Number of messages.
 * @param[out]    messages Message array.
 * @ingroup       functions
 */
idl_api void
idl_compilation_result_get_messages(idl_compilation_result_t compilation_result,
                                    idl_uint32_t* message_count,
                                    idl_message_t* messages);

/** @} */

IDL_END

#endif /* IDL_RESULTS_H */
