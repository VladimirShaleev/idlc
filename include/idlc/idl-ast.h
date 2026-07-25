/**
 * @file      idl-ast.h
 * @brief     TODO.
 * @details   TODO.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @ingroup   files
 * @copyright MIT License
 */
#ifndef IDL_AST_H
#define IDL_AST_H

#include "idl-version.h"
#include "idl-types.h"

IDL_BEGIN

/**
 * @brief   TODO.
 * @details TODO.
 * @ingroup enums
 */
typedef enum
{
    IDL_AST_NODE_TYPE_DECL               = 0, /**< TODO */
    IDL_AST_NODE_TYPE_API                = 1, /**< TODO */
    IDL_AST_NODE_TYPE_IMPORT             = 2, /**< TODO */
    IDL_AST_NODE_TYPE_ENUM               = 3, /**< TODO */
    IDL_AST_NODE_TYPE_CONST              = 4, /**< TODO */
    IDL_AST_NODE_TYPE_STRUCT             = 5, /**< TODO */
    IDL_AST_NODE_TYPE_FIELD              = 6, /**< TODO */
    IDL_AST_NODE_TYPE_FUNC               = 7, /**< TODO */
    IDL_AST_NODE_TYPE_ARG                = 8, /**< TODO */
    IDL_AST_NODE_TYPE_DECL_REF           = 9, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL            = 10, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL_STR        = 11, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL_INT        = 12, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL_BOOL       = 13, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL_FLOAT      = 14, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR               = 15, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_FLAGS         = 16, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_HEX           = 17, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_VALUE         = 18, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_TYPE          = 19, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_CNAME         = 20, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_TOKENIZER     = 21, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_ARRAY         = 22, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_SINGLE        = 23, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_REF           = 24, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_CONST         = 25, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_OPTIONAL      = 26, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_VERSION       = 27, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC           = 28, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF     = 29, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL    = 30, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR    = 31, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT = 32, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE   = 33, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_BUILTIN       = 34, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_MAX_ENUM      = 35, /**< TODO */
    IDL_AST_NODE_TYPE_TYPE               = 36, /**< TODO */
    IDL_AST_NODE_TYPE_TRIVIAL_TYPE       = 37, /**< TODO */
    IDL_AST_NODE_TYPE_VOID               = 38, /**< TODO */
    IDL_AST_NODE_TYPE_DATA               = 39, /**< TODO */
    IDL_AST_NODE_TYPE_CHAR               = 40, /**< TODO */
    IDL_AST_NODE_TYPE_STR                = 41, /**< TODO */
    IDL_AST_NODE_TYPE_BOOL               = 42, /**< TODO */
    IDL_AST_NODE_TYPE_INTEGER_TYPE       = 43, /**< TODO */
    IDL_AST_NODE_TYPE_INT_8              = 44, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_8             = 45, /**< TODO */
    IDL_AST_NODE_TYPE_INT_16             = 46, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_16            = 47, /**< TODO */
    IDL_AST_NODE_TYPE_INT_32             = 48, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_32            = 49, /**< TODO */
    IDL_AST_NODE_TYPE_INT_64             = 50, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_64            = 51, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_TYPE         = 52, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_32           = 53, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_64           = 54, /**< TODO */
    IDL_AST_NODE_TYPE_MAX_ENUM           = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_ast_node_type_t;

/**
 * @brief   TODO.
 * @details TODO.
 * @ingroup enums
 */
typedef enum
{
    IDL_AST_NODE_STATE_NONE_BIT                 = 0x00, /**< TODO */
    IDL_AST_NODE_STATE_EVAULATED_BIT            = 0x01, /**< TODO */
    IDL_AST_NODE_STATE_BUILD_ERROR_BIT          = 0x02, /**< TODO */
    IDL_AST_NODE_STATE_FORWARD_DECL_BIT         = 0x04, /**< TODO */
    IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT    = 0x08, /**< TODO */
    IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT = 0x10, /**< TODO */
    IDL_AST_NODE_STATE_MULTILINE_DOC_BIT        = 0x20, /**< TODO */
    IDL_AST_NODE_STATE_MAX_ENUM                 = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_ast_node_state_flags_t;
IDL_FLAGS(idl_ast_node_state_flags_t)

/**
 * @brief   TODO.
 * @details TODO.
 * @ingroup structs
 */
typedef struct
{
    idl_utf8_t   filename; /**< TODO */
    idl_uint16_t line; /**< TODO */
    idl_uint16_t column; /**< TODO */
} idl_ast_location_t;

IDL_END

#endif /* IDL_AST_H */
