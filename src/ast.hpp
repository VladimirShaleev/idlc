#ifndef AST_HPP
#define AST_HPP

#include "idl.hpp"
#include "string_pool.hpp"

namespace idl {

typedef idl_ast_node_h ASTNodeHandle;
typedef idl_ast_node_type_t ASTNodeType;
typedef idl_ast_node_state_flags_t ASTNodeFlags;

static constexpr ASTNodeType IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF = ASTNodeType(-1);

static constexpr ASTNodeHandle HandleNone{ 0xFFFF };

inline bool operator==(const ASTNodeHandle& lhs, const ASTNodeHandle& rhs) noexcept {
    return lhs.handle == rhs.handle;
}

inline bool operator!=(const ASTNodeHandle& lhs, const ASTNodeHandle& rhs) noexcept {
    return lhs.handle != rhs.handle;
}

template <ASTNodeType Type>
struct Tag {
    static constexpr ASTNodeType type = Type;
};

struct ASTLocation {
    String filename{};
    uint16_t column{};
    uint16_t line{};
};

struct ASTName {
    String name;
    String fullname;
};

struct ASTDeclRef {
    String symbol;
    ASTNodeHandle handle;
};

struct ASTNode {
    ASTLocation location;
    int8_t type;
    uint8_t flags;
    ASTNodeHandle parent;
    ASTNodeHandle sibling;
    ASTNodeHandle child;

    union {
        String valueStr;
        uint64_t valueInt;
        double valueFloat;
        bool valueBool;
        ASTName name;
        ASTDeclRef valueDeclRef;
    };

    ASTNode() noexcept : valueInt(0) {
    }
};

inline bool isNodeType(const ASTNode* node, ASTNodeType type) noexcept {
    if (node) {
        if (node->type == type) {
            return true;
        }
        switch (type) {
            case IDL_AST_NODE_TYPE_DECL:
                return (node->type >= IDL_AST_NODE_TYPE_DECL && node->type < IDL_AST_NODE_TYPE_DECL_REF) ||
                       node->type >= IDL_AST_NODE_TYPE_TYPE;
            case IDL_AST_NODE_TYPE_LITERAL:
                return node->type >= IDL_AST_NODE_TYPE_LITERAL && node->type < IDL_AST_NODE_TYPE_ATTR;
            case IDL_AST_NODE_TYPE_ATTR:
                return node->type >= IDL_AST_NODE_TYPE_ATTR && node->type < IDL_AST_NODE_TYPE_TYPE;
            case IDL_AST_NODE_TYPE_ATTR_DOC:
                return node->type >= IDL_AST_NODE_TYPE_ATTR_DOC && node->type < IDL_AST_NODE_TYPE_TYPE;
            case IDL_AST_NODE_TYPE_TYPE:
                return node->type >= IDL_AST_NODE_TYPE_TYPE || node->type == IDL_AST_NODE_TYPE_ENUM;
            case IDL_AST_NODE_TYPE_TRIVIAL_TYPE:
                return node->type >= IDL_AST_NODE_TYPE_TRIVIAL_TYPE;
            case IDL_AST_NODE_TYPE_INTEGER_TYPE:
                return node->type >= IDL_AST_NODE_TYPE_INTEGER_TYPE && node->type < IDL_AST_NODE_TYPE_FLOAT_TYPE;
            case IDL_AST_NODE_TYPE_FLOAT_TYPE:
                return node->type >= IDL_AST_NODE_TYPE_FLOAT_TYPE;
            default:
                break;
        }
    }
    return false;
}

} // namespace idl

#endif
