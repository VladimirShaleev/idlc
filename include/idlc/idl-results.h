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
    IDL_STATUS_W1001    = 1001, /**< Missing 'author' attribute. */
    IDL_STATUS_W1002    = 1002, /**< Missing 'copyright' attribute. */
    IDL_STATUS_E2001    = 2001, /**< Unexpected character. */
    IDL_STATUS_E2002    = 2002, /**< Tabs are not allowed. */
    IDL_STATUS_E2003    = 2003, /**< The name or type must start with a capital letter. */
    IDL_STATUS_E2004    = 2004, /**< There can only be one api declaration. */
    IDL_STATUS_E2005    = 2005, /**< There is no documentation in the declaration. */
    IDL_STATUS_E2006    = 2006, /**< Documentation cannot be an empty string. */
    IDL_STATUS_E2007    = 2007, /**< The brief should only be listed once in the documentation. */
    IDL_STATUS_E2008    = 2008, /**< The detail should only be listed once in the documentation. */
    IDL_STATUS_E2009    = 2009, /**< The copyright should only be listed once in the documentation. */
    IDL_STATUS_E2010    = 2010, /**< The license should only be listed once in the documentation. */
    IDL_STATUS_E2011    = 2011, /**< Unknown error. */
    IDL_STATUS_E2012    = 2012, /**< The .idl file must start with the 'api' element. */
    IDL_STATUS_E2013    = 2013, /**< Attribute cannot be duplicated. */
    IDL_STATUS_E2014    = 2014, /**< The attribute is not valid in the context of use. */
    IDL_STATUS_E2015    = 2015, /**< Unknown attribute. */
    IDL_STATUS_E2016    = 2016, /**< Attribute must specify at least one argument. */
    IDL_STATUS_E2017    = 2017, /**< Argument in attribute 'platform' is not allowed. */
    IDL_STATUS_E2018    = 2018, /**< Argument in attribute 'platform' cannot be duplicated. */
    IDL_STATUS_E2019    = 2019, /**< Inline documentation only [detail] description is allowed. */
    IDL_STATUS_E2020    = 2020, /**< Invalid attribute in documentation. */
    IDL_STATUS_E2021    = 2021, /**< It is acceptable to use either documentation or inline documentation, but not both. */
    IDL_STATUS_E2022    = 2022, /**< Constants can only be added to an enumeration type. */
    IDL_STATUS_E2023    = 2023, /**< The 'value' attribute must specify the value in the argument. */
    IDL_STATUS_E2024    = 2024, /**< The 'value' attribute must contain only one value. */
    IDL_STATUS_E2025    = 2025, /**< The 'value' attribute must specify an integer. */
    IDL_STATUS_E2026    = 2026, /**< An enumeration must contain at least one constant. */
    IDL_STATUS_E2027    = 2027, /**< Fields can only be added to a structured type. */
    IDL_STATUS_E2028    = 2028, /**< The 'type' attribute must specify the type in the argument. */
    IDL_STATUS_E2029    = 2029, /**< The 'type' attribute must contain only one type. */
    IDL_STATUS_E2030    = 2030, /**< Symbol redefinition. */
    IDL_STATUS_E2031    = 2031, /**< Enumeration constants can only be specified as integers or enum consts. */
    IDL_STATUS_E2032    = 2032, /**< Symbol definition not found. */
    IDL_STATUS_E2033    = 2033, /**< A constant cannot refer to itself when evaluated. */
    IDL_STATUS_E2034    = 2034, /**< Constants can only refer to other constants when evaluated. */
    IDL_STATUS_E2035    = 2035, /**< Declaration is not a type. */
    IDL_STATUS_E2036    = 2036, /**< Enumeration constant can only be of type 'Int32'. */
    IDL_STATUS_E2037    = 2037, /**< Identifiers are case sensitive. */
    IDL_STATUS_E2038    = 2038, /**< Constant cannot go beyond the range of 'Int32' [-2147483648, 2147483647]. */
    IDL_STATUS_E2039    = 2039, /**< Constant was duplicated. */
    IDL_STATUS_E2040    = 2040, /**< Cyclic dependence of constant. */
    IDL_STATUS_E2041    = 2041, /**< Could not find file for import. */
    IDL_STATUS_E2042    = 2042, /**< Failed to open file. */
    IDL_STATUS_E2043    = 2043, /**< Methods can only be added to a interface type. */
    IDL_STATUS_E2044    = 2044, /**< Arguments can only be added to a method, function or callback. */
    IDL_STATUS_E2045    = 2045, /**< Out of memory. */
    IDL_STATUS_E2046    = 2046, /**< Static method cannot include argument' with attribute 'this'. */
    IDL_STATUS_E2047    = 2047, /**< Constructor cannot include argument with attribute 'this'. */
    IDL_STATUS_E2048    = 2048, /**< Method must include one argument with the 'this' attribute. */
    IDL_STATUS_E2049    = 2049, /**< The 'get' attribute must specify a reference to the method in the argument. */
    IDL_STATUS_E2050    = 2050, /**< The 'set' attribute must specify a reference to the method in the argument. */
    IDL_STATUS_E2051    = 2051, /**< Argument of method cannot be of type 'Void'. */
    IDL_STATUS_E2052    = 2052, /**< The property must contain at least the 'get' attribute or the 'set' attribute or both. */
    IDL_STATUS_E2053    = 2053, /**< Getter must be a method. */
    IDL_STATUS_E2054    = 2054, /**< A property getter cannot reference a method from another interface. */
    IDL_STATUS_E2055    = 2055, /**< If the getter method is static, then the property must also be static, and vice versa. */
    IDL_STATUS_E2056    = 2056, /**< A static getter method must not have arguments. */
    IDL_STATUS_E2057    = 2057, /**< A getter method must have one argument. */
    IDL_STATUS_E2058    = 2058, /**< Getter method cannot return 'Void'. */
    IDL_STATUS_E2059    = 2059, /**< Setter must be a method. */
    IDL_STATUS_E2060    = 2060, /**< If the setter method is static, then the property must also be static, and vice versa. */
    IDL_STATUS_E2061    = 2061, /**< A property setter cannot reference a method from another interface. */
    IDL_STATUS_E2062    = 2062, /**< A static setter method must have one argument. */
    IDL_STATUS_E2063    = 2063, /**< A setter method must have two arguments. */
    IDL_STATUS_E2064    = 2064, /**< The return type of the getter method is different from the argument type of the setter method. */
    IDL_STATUS_E2065    = 2065, /**< The property type does not match the return type of the getter method. */
    IDL_STATUS_E2066    = 2066, /**< The property type does not match the setter method argument type. */
    IDL_STATUS_E2067    = 2067, /**< Failed to create file. */
    IDL_STATUS_E2068    = 2068, /**< Field 'of struct cannot be of type 'Void'. */
    IDL_STATUS_E2069    = 2069, /**< The handle type must be specified. */
    IDL_STATUS_E2070    = 2070, /**< The handle type must be struct. */
    IDL_STATUS_E2071    = 2071, /**< The structure specified in the handle type must be marked with the 'handle' attribute. */
    IDL_STATUS_E2072    = 2072, /**< It is not possible to add the 'noerror' attribute to the constant because the enum does not have the 'errorcode' attribute. */
    IDL_STATUS_E2073    = 2073, /**< Function argument cannot be marked with the 'this' attribute. */
    IDL_STATUS_E2074    = 2074, /**< Argument of function cannot be of type 'Void'. */
    IDL_STATUS_E2075    = 2075, /**< The 'cname' attribute must specify a string in the argument. */
    IDL_STATUS_E2076    = 2076, /**< The 'array' attribute must specify a size in the argument. */
    IDL_STATUS_E2077    = 2077, /**< Fixed size array of structure must be of size 1 or more. */
    IDL_STATUS_E2078    = 2078, /**< The 'array' attribute of the must point to a field of the structure or set fixed size value. */
    IDL_STATUS_E2079    = 2079, /**< The reference to the dynamic size array is located outside the visibility of the structure. */
    IDL_STATUS_E2080    = 2080, /**< The 'array' attribute for array must point to an integer field for a dynamic array. */
    IDL_STATUS_E2081    = 2081, /**< An struct must contain at least one field. */
    IDL_STATUS_E2082    = 2082, /**< There can be only one argument with the 'userdata' attribute. */
    IDL_STATUS_E2083    = 2083, /**< Callback argument cannot be marked with the 'this' attribute. */
    IDL_STATUS_E2084    = 2084, /**< There can be only one argument with the 'result' attribute. */
    IDL_STATUS_E2085    = 2085, /**< The function to convert an error code to a string must return a string and take one argument (the error code). */
    IDL_STATUS_E2086    = 2086, /**< The method for incrementing the reference counter ('refinc' attribute) of an object must be non-static and take one argument 'this'. */
    IDL_STATUS_E2087    = 2087, /**< The method for destroy of an object must be non-static and take one argument 'this'. */
    IDL_STATUS_E2088    = 2088, /**< There can only be one method to increment reference counter. */
    IDL_STATUS_E2089    = 2089, /**< There can only be one method to destroy object. */
    IDL_STATUS_E2090    = 2090, /**< Events can only be added to a interface type. */
    IDL_STATUS_E2091    = 2091, /**< The event must contain at least the 'get' attribute or the 'set' attribute or both. */
    IDL_STATUS_E2092    = 2092, /**< Event getter from refers to a method from another interface. */
    IDL_STATUS_E2093    = 2093, /**< If the getter method is static, then the event must also be static, and vice versa. */
    IDL_STATUS_E2094    = 2094, /**< Static getter for event must have no arguments or one argument 'userdata'. */
    IDL_STATUS_E2095    = 2095, /**< Getter for event must have one arguments or two arguments 'this' and 'userdata'. */
    IDL_STATUS_E2096    = 2096, /**< Event setter from refers to a method from another interface. */
    IDL_STATUS_E2097    = 2097, /**< If the setter method is static, then the event must also be static, and vice versa. */
    IDL_STATUS_E2098    = 2098, /**< Static setter for event must have one argument or setter and 'userdata' arguments. */
    IDL_STATUS_E2099    = 2099, /**< Setter for event must have two arguments 'this' and 'setter' or three arguments 'this', 'setter' and 'userdata'. */
    IDL_STATUS_E2100    = 2100, /**< The event type does not match the return type of the getter method. */
    IDL_STATUS_E2101    = 2101, /**< The event type does not match the setter method argument type. */
    IDL_STATUS_E2102    = 2102, /**< The argument of a method, function, or callback cannot be a fixed-size array. */
    IDL_STATUS_E2103    = 2103, /**< The reference to the dynamic size array is located outside the visibility of the method. */
    IDL_STATUS_E2104    = 2104, /**< The 'array' attribute of the must point to a argument of the method. */
    IDL_STATUS_E2105    = 2105, /**< The reference to the dynamic size array is located outside the visibility of the function. */
    IDL_STATUS_E2106    = 2106, /**< The 'array' attribute of the must point to a argument of the function. */
    IDL_STATUS_E2107    = 2107, /**< The reference to the dynamic size array is located outside the visibility of the callback. */
    IDL_STATUS_E2108    = 2108, /**< The 'array' attribute of the must point to a argument of the callback. */
    IDL_STATUS_E2109    = 2109, /**< The 'tokenizer' attribute must specify a indices string in the argument. */
    IDL_STATUS_E2110    = 2110, /**< The 'version' attribute must specify a semver in the argument. */
    IDL_STATUS_E2111    = 2111, /**< The declaration does not have a brief ('brief' attribute) or detailed description ('detail' attribute). */
    IDL_STATUS_E2112    = 2112, /**< The 'datasize' attribute must specify a size in the argument. */
    IDL_STATUS_E2113    = 2113, /**< The 'datasize' attribute of the must point to a field of the structure. */
    IDL_STATUS_E2114    = 2114, /**< The 'datasize' attribute of the must point to an integer field to specify the buffer size. */
    IDL_STATUS_E2115    = 2115, /**< The 'datasize' attribute of the must point to a argument of the method. */
    IDL_STATUS_E2116    = 2116, /**< The 'datasize' attribute of the must point to a argument of the function. */
    IDL_STATUS_E2117    = 2117, /**< The 'datasize' attribute of the must point to a argument of the callback. */
    IDL_STATUS_E2118    = 2118, /**< The reference to the size buffer is located outside the visibility of the structure. */
    IDL_STATUS_E2119    = 2119, /**< The 'datasize' attribute is only applicable to fields of type "Data" or "ConstData". */
    IDL_STATUS_E2120    = 2120, /**< The reference to the size buffer is located outside the visibility of the callback. */
    IDL_STATUS_E2121    = 2121, /**< The 'datasize' attribute is only applicable to arg of type "Data" or "ConstData". */
    IDL_STATUS_E2122    = 2122, /**< The reference to the size buffer is located outside the visibility of the function. */
    IDL_STATUS_E2123    = 2123, /**< The reference to the size buffer is located outside the visibility of the method. */
    IDL_STATUS_E2124    = 2124, /**< Can only specify either the 'datasize' or 'array' attribute, but not both. */
    IDL_STATUS_E2125    = 2125, /**< Cannot contain attribute 'errorcode'. */
    IDL_STATUS_E2126    = 2126, /**< The 'refinc' attribute can only contain a method. */
    IDL_STATUS_E2127    = 2127, /**< The 'destroy' attribute can only contain a method. */
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
