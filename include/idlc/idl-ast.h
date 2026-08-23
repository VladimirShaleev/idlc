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
    IDL_AST_NODE_TYPE_ATTR_MAX_ENUM      = 18, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_COUNT_ENUMS   = 19, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_VALUE         = 20, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_TYPE          = 21, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_CNAME         = 22, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_CCONV         = 23, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_CFORMAT       = 24, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_TOKENIZER     = 25, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_ARRAY         = 26, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_SINGLE        = 27, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_STD_TYPES     = 28, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE     = 29, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_REF           = 30, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_CONST         = 31, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_OPTIONAL      = 32, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_VERSION       = 33, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC           = 34, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF     = 35, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL    = 36, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR    = 37, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT = 38, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE   = 39, /**< TODO */
    IDL_AST_NODE_TYPE_TYPE               = 40, /**< TODO */
    IDL_AST_NODE_TYPE_TRIVIAL_TYPE       = 41, /**< TODO */
    IDL_AST_NODE_TYPE_VOID               = 42, /**< TODO */
    IDL_AST_NODE_TYPE_DATA               = 43, /**< TODO */
    IDL_AST_NODE_TYPE_CHAR               = 44, /**< TODO */
    IDL_AST_NODE_TYPE_STR                = 45, /**< TODO */
    IDL_AST_NODE_TYPE_BOOL               = 46, /**< TODO */
    IDL_AST_NODE_TYPE_INTEGER_TYPE       = 47, /**< TODO */
    IDL_AST_NODE_TYPE_INT_8              = 48, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_8             = 49, /**< TODO */
    IDL_AST_NODE_TYPE_INT_16             = 50, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_16            = 51, /**< TODO */
    IDL_AST_NODE_TYPE_INT_32             = 52, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_32            = 53, /**< TODO */
    IDL_AST_NODE_TYPE_INT_64             = 54, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_64            = 55, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_TYPE         = 56, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_32           = 57, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_64           = 58, /**< TODO */
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
    IDL_AST_NODE_STATE_BUILTIN_BIT              = 0x40, /**< TODO */
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
