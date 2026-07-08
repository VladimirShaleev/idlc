#ifndef AST_HPP
#define AST_HPP

#include "idl.hpp"
#include "string_pool.hpp"

namespace idl {

enum class ASTNodeType : uint8_t {
    Tombstone,

    Decl,
    Api,
    Import,
    Enum,
    Const,

    DeclRef,

    Literal,
    LiteralStr,
    LiteralInt,
    LiteralBool,
    LiteralFloat,

    Attr,
    AttrFlags,
    AttrHex,
    AttrValue,
    AttrType,
    AttrCName,
    AttrTokenizer,
    AttrOrder,
    AttrSingle,
    AttrVersion,

    AttrDoc,
    AttrDocBrief,
    AttrDocDetail,
    AttrDocAuthor,
    AttrDocCopyright,
    AttrDocLicense,

    TrivialType,
    Void,
    Data,
    Char,
    Str,
    Bool,
    IntegerType,
    Int8,
    Uint8,
    Int16,
    Uint16,
    Int32,
    Uint32,
    Int64,
    Uint64,
    FloatType,
    Float32,
    Float64
};

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
    String name{};
    String nameLower{};
};

struct ASTNodeHandle {
    uint16_t handle;
};

static constexpr ASTNodeHandle NodeHandleNone{ 0xFFFF };

inline bool operator==(const ASTNodeHandle& lhs, const ASTNodeHandle& rhs) noexcept {
    return lhs.handle == rhs.handle;
}

inline bool operator!=(const ASTNodeHandle& lhs, const ASTNodeHandle& rhs) noexcept {
    return lhs.handle != rhs.handle;
}

enum ASTNodeFlags {
    ASTNODE_EVAULATED    = 1,
    ASTNODE_BUILD_ERROR  = 2,
    ASTNODE_FORWARD_DECL = 4
};

struct ASTNode {
    ASTLocation location;
    ASTNodeType type;
    uint8_t flags;
    ASTNodeHandle parent;
    ASTNodeHandle sibling;
    ASTNodeHandle child;

    union {
        String valueStr;
        uint64_t valueInt;
        double valueFloat;
        bool valueBool;
        ASTNodeHandle valueHandle;
    };

    ASTNode() noexcept : valueStr{} {
    }
};

inline bool astNodeIs(const ASTNode* node, ASTNodeType type) noexcept {
    if (node) {
        if (node->type == type) {
            return true;
        }
        switch (type) {
            case ASTNodeType::Decl:
                return (node->type >= ASTNodeType::Decl && node->type < ASTNodeType::DeclRef) ||
                       node->type >= ASTNodeType::TrivialType;
            case ASTNodeType::Literal:
                return node->type >= ASTNodeType::Literal && node->type < ASTNodeType::Attr;
            case ASTNodeType::Attr:
                return node->type >= ASTNodeType::Attr && node->type < ASTNodeType::TrivialType;
            case ASTNodeType::AttrDoc:
                return node->type >= ASTNodeType::AttrDoc && node->type < ASTNodeType::TrivialType;
            case ASTNodeType::TrivialType:
                return node->type >= ASTNodeType::TrivialType;
            case ASTNodeType::IntegerType:
                return node->type >= ASTNodeType::IntegerType && node->type < ASTNodeType::FloatType;
            case ASTNodeType::FloatType:
                return node->type >= ASTNodeType::FloatType;
            default:
                break;
        }
    }
    return false;
}

} // namespace idl

#endif
