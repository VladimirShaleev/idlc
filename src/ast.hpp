#ifndef AST_HPP
#define AST_HPP

#include <algorithm>
#include <string>
#include <vector>

#include <magic_enum/magic_enum.hpp>

#include "location.hh"

struct ASTNode {
    virtual ~ASTNode() = default;
    ASTNode* parent{};
    idl::location location{};
};

struct ASTLiteral : ASTNode {};

struct ASTLiteralStr : ASTLiteral {
    std::string value;
};

struct ASTDoc : ASTNode {
    std::vector<ASTNode*> brief;
    std::vector<ASTNode*> detail;
    std::vector<ASTNode*> copyright;
    std::vector<ASTNode*> license;
    std::vector<std::vector<ASTNode*>> authors;
};

enum struct TargetPlatfrom {
    Windows,
    Linux,
    MacOS,
    Web,
    Android,
    iOS
};

struct ASTAttr : ASTNode {
    enum Type {
        Platform,
        Flags,
        Hex,
        Value
    };

    struct Arg {
        union {
            int64_t value;
            TargetPlatfrom platform;
        };
    };

    Type type;
    std::vector<Arg> args;

    static std::string typeStr(Type type) {
        std::string str = magic_enum::enum_name(type).data();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        return str;
    }

    std::string typeStr() const {
        return typeStr(type);
    }
};

struct ASTDecl : ASTNode {
    std::string name;
    std::vector<ASTAttr*> attrs;
    ASTDoc* doc{};

    template <ASTAttr::Type Type>
    const ASTAttr* findAttr() const noexcept {
        auto it = std::find_if(attrs.begin(), attrs.end(), [](auto attr) {
            return attr->type == Type;
        });
        return it != attrs.end() ? *it : nullptr;
    }
};

struct ASTDeclRef : ASTNode {
    std::string name;
    ASTDecl* decl{};
};

struct ASTType : ASTDecl {};

struct ASTEnumConst : ASTDecl {
    int32_t value{};
};

struct ASTEnum : ASTType {
    std::vector<ASTEnumConst*> consts;
};

struct ASTApi : ASTDecl {
    std::vector<ASTEnum*> enums;
};

#endif
