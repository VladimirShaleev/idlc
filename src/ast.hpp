#ifndef AST_HPP
#define AST_HPP

#include "idl.hpp"
#include "string_pool.hpp"

namespace idl {

typedef idl_ast_node_h ASTNodeHandle;
typedef idl_ast_node_type_t ASTNodeType;
typedef idl_ast_node_state_flags_t ASTNodeFlags;

static constexpr ASTNodeType IDL_AST_NODE_TYPE_PRIVATE               = ASTNodeType(uint8_t(IDL_AST_NODE_TYPE_FLOAT_64 + 1));
static constexpr ASTNodeType IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF = ASTNodeType(uint8_t(IDL_AST_NODE_TYPE_PRIVATE + 0));

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
        int64_t valueInt;
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
        const auto nodeType = node->type;
        if (nodeType == type) {
            return true;
        }
        switch (type) {
            case IDL_AST_NODE_TYPE_DECL:
                return (nodeType >= IDL_AST_NODE_TYPE_DECL && nodeType < IDL_AST_NODE_TYPE_DECL_REF) ||
                       nodeType >= IDL_AST_NODE_TYPE_TYPE && nodeType <= IDL_AST_NODE_TYPE_FLOAT_64;
            case IDL_AST_NODE_TYPE_LITERAL:
                return nodeType >= IDL_AST_NODE_TYPE_LITERAL && nodeType < IDL_AST_NODE_TYPE_ATTR;
            case IDL_AST_NODE_TYPE_ATTR:
                return nodeType >= IDL_AST_NODE_TYPE_ATTR && nodeType < IDL_AST_NODE_TYPE_TYPE;
            case IDL_AST_NODE_TYPE_ATTR_DOC:
                return nodeType >= IDL_AST_NODE_TYPE_ATTR_DOC && nodeType < IDL_AST_NODE_TYPE_TYPE;
            case IDL_AST_NODE_TYPE_TYPE:
                return nodeType >= IDL_AST_NODE_TYPE_TYPE && nodeType <= IDL_AST_NODE_TYPE_FLOAT_64 || nodeType == IDL_AST_NODE_TYPE_ENUM ||
                       nodeType == IDL_AST_NODE_TYPE_STRUCT;
            case IDL_AST_NODE_TYPE_TRIVIAL_TYPE:
                return nodeType >= IDL_AST_NODE_TYPE_TRIVIAL_TYPE && nodeType <= IDL_AST_NODE_TYPE_FLOAT_64;
            case IDL_AST_NODE_TYPE_INTEGER_TYPE:
                return nodeType >= IDL_AST_NODE_TYPE_INTEGER_TYPE && nodeType < IDL_AST_NODE_TYPE_FLOAT_TYPE;
            case IDL_AST_NODE_TYPE_FLOAT_TYPE:
                return nodeType >= IDL_AST_NODE_TYPE_FLOAT_TYPE && nodeType <= IDL_AST_NODE_TYPE_FLOAT_64;
            default:
                break;
        }
    }
    return false;
}

} // namespace idl

#endif
