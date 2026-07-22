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

#include "idl-ast.h"

IDL_BEGIN

/**
 * @brief   Result codes.
 * @details Enumeration of result codes.
 * @ingroup enums
 */
typedef enum
{
    IDL_RESULT_SUCCESS                = 0, /**< Indicates success (this is not an error). */
    IDL_RESULT_ERROR_UNKNOWN          = 1, /**< Unknown error. */
    IDL_RESULT_ERROR_OUT_OF_MEMORY    = 2, /**< Out of memory. */
    IDL_RESULT_ERROR_INVALID_ARG      = 3, /**< Invalid argument. */
    IDL_RESULT_ERROR_FILE_CREATE      = 4, /**< Failed to create file. */
    IDL_RESULT_ERROR_COMPILATION      = 5, /**< Compilation failed. */
    IDL_RESULT_ERROR_NOT_SUPPORTED    = 6, /**< Not supporeted. */
    IDL_RESULT_ERROR_SOURCE_NOT_FOUND = 7, /**< Source not found. */
    IDL_RESULT_MAX_ENUM               = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_result_t;

/**
 * @brief   Compilation statuses.
 * @details This enumeration contains warnings and errors that may occur during compilation.
 * @ingroup enums
 */
typedef enum
{
    IDL_STATUS_N1001    = 1001, /**< Unnecessary parentheses for a parameterless attribute. */
    IDL_STATUS_N1002    = 1002, /**< Unnecessary parentheses for empty attribute list. */
    IDL_STATUS_N1003    = 1003, /**< Unnecessary explicit attribute [brief] in documentation. */
    IDL_STATUS_N1004    = 1004, /**< Unnecessary explicit attribute [detail] in inline documentation. */
    IDL_STATUS_W2001    = 2001, /**< The declaration is missing an attribute. */
    IDL_STATUS_W2002    = 2002, /**< Repeated import. */
    IDL_STATUS_W2003    = 2003, /**< The constant refers to a constant declared below. */
    IDL_STATUS_W2004    = 2004, /**< Value out of range. */
    IDL_STATUS_W2005    = 2005, /**< Special character expected after backslash. */
    IDL_STATUS_W2006    = 2006, /**< The field type has the declaration type declared below. */
    IDL_STATUS_W2007    = 2007, /**< Implicit conversion from an integer type to a floating-point type. */
    IDL_STATUS_E3001    = 3001, /**< Syntax error. */
    IDL_STATUS_E3002    = 3002, /**< Argument parsing error. */
    IDL_STATUS_E3003    = 3003, /**< The [version] attribute must have three required integer parameters, such as version(1, 2, 3) or version("string"). */
    IDL_STATUS_E3004    = 3004, /**< Version values must be between 0 and 255. */
    IDL_STATUS_E3005    = 3005, /**< An invalid attribute was specified for the declaration. */
    IDL_STATUS_E3007    = 3007, /**< Attribute duplication. */
    IDL_STATUS_E3008    = 3008, /**< The attribute must not have arguments. */
    IDL_STATUS_E3009    = 3009, /**< String closing character not found. */
    IDL_STATUS_E3010    = 3010, /**< API Redeclaration. */
    IDL_STATUS_E3011    = 3011, /**< The first declaration in the description should always begin with the 'api' declaration. */
    IDL_STATUS_E3012    = 3012, /**< Symbol redefinition. */
    IDL_STATUS_E3013    = 3013, /**< Unknown attribute. */
    IDL_STATUS_E3014    = 3014, /**< Attribute must contain one or more arguments. */
    IDL_STATUS_E3015    = 3015, /**< Unknown attribute in the documentation. */
    IDL_STATUS_E3016    = 3016, /**< The documentation string cannot be empty. */
    IDL_STATUS_E3017    = 3017, /**< The body of a multi-line comment '@ ```' must be separated by a new line. */
    IDL_STATUS_E3018    = 3018, /**< Inline documentation only [detail] description is allowed. */
    IDL_STATUS_E3019    = 3019, /**< A 'field' can be defined only for an 'struct'. */
    IDL_STATUS_E3020    = 3020, /**< Tabs are not allowed. */
    IDL_STATUS_E3021    = 3021, /**< Could not find file for import. */
    IDL_STATUS_E3022    = 3022, /**< Failed to open file. */
    IDL_STATUS_E3023    = 3023, /**< A 'const' can be defined only for an 'enum'. */
    IDL_STATUS_E3024    = 3024, /**< Structure must contain at least one field. */
    IDL_STATUS_E3025    = 3025, /**< Arguments for the [value] attribute must be literals or declaration reference. */
    IDL_STATUS_E3026    = 3026, /**< All literals in the [value] attribute must be of the same type. */
    IDL_STATUS_E3027    = 3027, /**< The [type] attribute argument can only refer to symbols. */
    IDL_STATUS_E3028    = 3028, /**< The [cname] attribute must contain a single string literal argument. */
    IDL_STATUS_E3029    = 3029, /**< The [cname] attribute must specify a name without spaces and punctuations. */
    IDL_STATUS_E3030    = 3030, /**< This field has the type of the structure declared below. */
    IDL_STATUS_E3031    = 3031, /**< Invalid tokenizer format string, a valid string looks like (2-^3-4). */
    IDL_STATUS_E3032    = 3032, /**< Integer tokenization parameters or a tokenizer string must be passed to the attribute [tokenizer]. */
    IDL_STATUS_E3033    = 3033, /**< The field type corresponds to the type of the structure in which it is contained. */
    IDL_STATUS_E3034    = 3034, /**< The declaration cannot be of type 'Void'. */
    IDL_STATUS_E3035    = 3035, /**< It is not possible to assign a literal to this type. */
    IDL_STATUS_E3036    = 3036, /**< Identifiers are case sensitive. */
    IDL_STATUS_E3037    = 3037, /**< Symbol definition not found. */
    IDL_STATUS_E3038    = 3038, /**< Constants can only refer to other constants when evaluated. */
    IDL_STATUS_E3039    = 3039, /**< A constant cannot refer to itself when evaluated. */
    IDL_STATUS_E3040    = 3040, /**< Enumeration constants can only be specified as integers or enum consts. */
    IDL_STATUS_E3041    = 3041, /**< Failed to calculate the constant. */
    IDL_STATUS_E3042    = 3042, /**< Cyclic dependence of constant. */
    IDL_STATUS_E3043    = 3043, /**< The [type] attribute must contain only one type. */
    IDL_STATUS_E3044    = 3044, /**< Enumeration can only of integers type. */
    IDL_STATUS_E3045    = 3045, /**< Enumeration must contain at least one constant. */
    IDL_STATUS_E3046    = 3046, /**< Unknown error. */
    IDL_STATUS_E3047    = 3047, /**< The name or type must start with a capital letter. */
    IDL_STATUS_E3048    = 3048, /**< Only literals and compile-time expressions (enumeration constants) can be used as default values. */
    IDL_STATUS_E3049    = 3049, /**< Cannot assign a constant of this type to a declaration with a different enumeration type. */
    IDL_STATUS_E3050    = 3050, /**< Attribute must contain one argument. */
    IDL_STATUS_E3051    = 3051, /**< The [array] size argument must be positive. */
    IDL_STATUS_E3052    = 3052, /**< The [array] attribute must refer to an integer declaration. */
    IDL_STATUS_E3053    = 3053, /**< The reference to the array size must be in the same scope. */
    IDL_STATUS_E3054    = 3054, /**< An array size reference cannot refer to its own declaration. */
    IDL_STATUS_E3055    = 3055, /**< Multiple default values can be assigned only to an array. */
    IDL_STATUS_E3056    = 3056, /**< Multiple enumeration constants can be assigned only to a flag enumeration. */
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
 * @brief     Checking if there were notes.
 * @details   Check if there were any notes during compilation.
 * @param[in] compilation_result Target compilation result instance.
 * @return    *TRUE*, if there are notes.
 * @ingroup   functions
 */
idl_api idl_bool_t
idl_compilation_result_has_notes(idl_compilation_result_t compilation_result);

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

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @return    TODO.
 * @ingroup   functions
 */
idl_api idl_ast_node_h
idl_compilation_result_get_api(idl_compilation_result_t compilation_result);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @ingroup   functions
 */
idl_api idl_ast_node_type_t
idl_compilation_result_get_node_type(idl_compilation_result_t compilation_result,
                                     idl_ast_node_h node);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @ingroup   functions
 */
idl_api idl_ast_node_state_flags_t
idl_compilation_result_get_node_state(idl_compilation_result_t compilation_result,
                                      idl_ast_node_h node);

/**
 * @brief      TODO
 * @details    TODO.
 * @param[in]  compilation_result Target compilation result instance.
 * @param[in]  node TODO
 * @param[out] location TODO
 * @ingroup    functions
 */
idl_api void
idl_compilation_result_get_node_location(idl_compilation_result_t compilation_result,
                                         idl_ast_node_h node,
                                         idl_ast_location_t* location);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @ingroup   functions
 */
idl_api idl_ast_node_h
idl_compilation_result_get_parent_node(idl_compilation_result_t compilation_result,
                                       idl_ast_node_h node);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @ingroup   functions
 */
idl_api idl_ast_node_h
idl_compilation_result_get_next_node(idl_compilation_result_t compilation_result,
                                     idl_ast_node_h node);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @ingroup   functions
 */
idl_api idl_ast_node_h
idl_compilation_result_get_child_node(idl_compilation_result_t compilation_result,
                                      idl_ast_node_h node);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @param[in] type TODO
 * @return    TODO.
 * @ingroup   functions
 */
idl_api idl_bool_t
idl_compilation_result_is_node_type(idl_compilation_result_t compilation_result,
                                    idl_ast_node_h node,
                                    idl_ast_node_type_t type);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @return    TODO.
 * @ingroup   functions
 */
idl_api idl_utf8_t
idl_compilation_result_get_node_value_str(idl_compilation_result_t compilation_result,
                                          idl_ast_node_h node);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @return    TODO.
 * @ingroup   functions
 */
idl_api idl_sint64_t
idl_compilation_result_get_node_value_int(idl_compilation_result_t compilation_result,
                                          idl_ast_node_h node);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @return    TODO.
 * @ingroup   functions
 */
idl_api idl_float64_t
idl_compilation_result_get_node_value_float(idl_compilation_result_t compilation_result,
                                            idl_ast_node_h node);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @return    TODO.
 * @ingroup   functions
 */
idl_api idl_bool_t
idl_compilation_result_get_node_value_bool(idl_compilation_result_t compilation_result,
                                           idl_ast_node_h node);

/**
 * @brief     TODO
 * @details   TODO.
 * @param[in] compilation_result Target compilation result instance.
 * @param[in] node TODO
 * @return    TODO.
 * @ingroup   functions
 */
idl_api idl_ast_node_h
idl_compilation_result_get_node_value_decl_ref(idl_compilation_result_t compilation_result,
                                               idl_ast_node_h node);

/** @} */

IDL_END

#endif /* IDL_RESULTS_H */
