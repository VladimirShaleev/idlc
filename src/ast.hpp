#ifndef AST_HPP
#define AST_HPP

#include <algorithm>
#include <string>
#include <vector>

#include "location.hh"

struct ASTNode;
struct ASTDecl;
struct ASTAttribute;
struct ASTProgram;
struct ASTNamespace;
struct ASTEnum;
struct ASTEnumConst;
struct ASTInterface;
struct ASTMethod;
struct ASTParameter;

struct ASTNode {
    virtual ~ASTNode() = default;
    ASTNode* parent{};
    idl::location location{};

    const ASTNamespace* ns() const noexcept;
};

struct ASTAttribute : ASTNode {
    enum Attribute {
        Platform,
        Flags,
        Hex,
        In,
        Out
    };

    Attribute type;
    std::vector<std::string> arguments;
};

struct ASTDecl : ASTNode {
    std::string name;
    std::vector<ASTAttribute*> attributes;

    [[nodiscard]] std::string type() const {
        auto parentDecl = dynamic_cast<const ASTDecl*>(parent);
        return parentDecl ? parentDecl->type() + "." + name : name;
    }

    [[nodiscard]] std::string typeLowercase() const {
        auto str = type();
        std::transform(str.cbegin(), str.cend(), str.begin(), [](const auto c) {
            return std::tolower(c);
        });
        return str;
    }
};

struct ASTType : ASTDecl {};

struct ASTDeclRef : ASTNode {
    std::string name;
    ASTDecl* decl{};

    [[nodiscard]] std::string nameLowercase() const {
        auto str = name;
        std::transform(str.cbegin(), str.cend(), str.begin(), [](const auto c) {
            return std::tolower(c);
        });
        return str;
    }
};

struct ASTProgram : ASTNode {
    std::vector<ASTNamespace*> namespaces;
};

struct ASTNamespace : ASTDecl {
    std::vector<ASTDecl*> declarations;
};

struct ASTEnum : ASTType {
    std::vector<ASTEnumConst*> constants;
};

struct ASTEnumConst : ASTDecl {
    int64_t value{};
};

struct ASTInterface : ASTType {
    std::vector<ASTMethod*> methods;
};

struct ASTMethod : ASTDecl {
    ASTDeclRef* returnTypeRef{};
    std::vector<ASTParameter*> parameters;
};

struct ASTParameter : ASTDecl {
    ASTDeclRef* typeRef{};
};

inline const ASTNamespace* ASTNode::ns() const noexcept {
    auto current = parent;
    while (current && dynamic_cast<const ASTNamespace*>(current) == nullptr) {
        current = current->parent;
    }
    return dynamic_cast<const ASTNamespace*>(current);
}

#endif
