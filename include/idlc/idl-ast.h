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
    IDL_AST_NODE_TYPE_DECL_REF           = 8, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL            = 9, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL_STR        = 10, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL_INT        = 11, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL_BOOL       = 12, /**< TODO */
    IDL_AST_NODE_TYPE_LITERAL_FLOAT      = 13, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR               = 14, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_FLAGS         = 15, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_HEX           = 16, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_VALUE         = 17, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_TYPE          = 18, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_CNAME         = 19, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_TOKENIZER     = 20, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_ARRAY         = 21, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_SINGLE        = 22, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_REF           = 23, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_CONST         = 24, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_OPTIONAL      = 25, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_VERSION       = 26, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC           = 27, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF     = 28, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL    = 29, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR    = 30, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT = 31, /**< TODO */
    IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE   = 32, /**< TODO */
    IDL_AST_NODE_TYPE_TYPE               = 33, /**< TODO */
    IDL_AST_NODE_TYPE_TRIVIAL_TYPE       = 34, /**< TODO */
    IDL_AST_NODE_TYPE_VOID               = 35, /**< TODO */
    IDL_AST_NODE_TYPE_DATA               = 36, /**< TODO */
    IDL_AST_NODE_TYPE_CHAR               = 37, /**< TODO */
    IDL_AST_NODE_TYPE_STR                = 38, /**< TODO */
    IDL_AST_NODE_TYPE_BOOL               = 39, /**< TODO */
    IDL_AST_NODE_TYPE_INTEGER_TYPE       = 40, /**< TODO */
    IDL_AST_NODE_TYPE_INT_8              = 41, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_8             = 42, /**< TODO */
    IDL_AST_NODE_TYPE_INT_16             = 43, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_16            = 44, /**< TODO */
    IDL_AST_NODE_TYPE_INT_32             = 45, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_32            = 46, /**< TODO */
    IDL_AST_NODE_TYPE_INT_64             = 47, /**< TODO */
    IDL_AST_NODE_TYPE_UINT_64            = 48, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_TYPE         = 49, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_32           = 50, /**< TODO */
    IDL_AST_NODE_TYPE_FLOAT_64           = 51, /**< TODO */
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
