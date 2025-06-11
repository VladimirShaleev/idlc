#ifndef AST_HPP
#define AST_HPP

#include <algorithm>
#include <string>
#include <vector>

#include "location.hh"

struct ASTNode;
struct ASTDecl;
struct ASTDeclRef;
struct ASTType;
struct ASTAttribute;
struct ASTProgram;
struct ASTNamespace;
struct ASTEnum;
struct ASTEnumConst;
struct ASTInterface;
struct ASTMethod;
struct ASTParameter;

#define AST_NODE_VISIT(Node)               \
    virtual bool visit(const Node* node) { \
        return false;                      \
    }

#define AST_NODE_ACCEPT                            \
    bool accept(Visitor& visitor) const override { \
        return visitor.visit(this);                \
    }

struct Visitor {
    virtual ~Visitor() = default;

    AST_NODE_VISIT(ASTDeclRef)
    AST_NODE_VISIT(ASTAttribute)
    AST_NODE_VISIT(ASTProgram)
    AST_NODE_VISIT(ASTNamespace)
    AST_NODE_VISIT(ASTEnum)
    AST_NODE_VISIT(ASTEnumConst)
    AST_NODE_VISIT(ASTInterface)
    AST_NODE_VISIT(ASTMethod)
    AST_NODE_VISIT(ASTParameter)
};

struct ASTNode {
    virtual ~ASTNode() = default;
    ASTNode* parent{};
    idl::location location{};

    const ASTNamespace* ns() const noexcept;

    virtual bool accept(Visitor& visitor) const = 0;
};

struct TargetPlatform {
    enum Type {
        Windows,
        Linux,
        MacOS,
        Web,
        Android,
        iOS
    };

    static constexpr const char* names[] = { "windows", "linux", "macos", "web", "android", "ios" };
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

    AST_NODE_ACCEPT
};

struct ASTDecl : ASTNode {
    std::string name;
    std::vector<ASTAttribute*> attributes;

    bool isPlatform(TargetPlatform::Type platform) const noexcept {
        auto it = std::find_if(attributes.cbegin(), attributes.cend(), [](const auto& item) {
            return item->type == ASTAttribute::Platform;
        });
        if (it == attributes.cend()) {
            return true;
        }
        for (const auto& arg : (*it)->arguments) {
            if (arg == TargetPlatform::names[platform]) {
                return true;
            }
        }
        return false;
    }

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

    AST_NODE_ACCEPT
};

struct ASTType : ASTDecl {};

struct ASTProgram : ASTNode {
    std::vector<ASTNamespace*> namespaces;

    AST_NODE_ACCEPT
};

struct ASTNamespace : ASTDecl {
    std::vector<ASTDecl*> declarations;

    AST_NODE_ACCEPT
};

struct ASTEnum : ASTType {
    std::vector<ASTEnumConst*> constants;

    bool isFlags() const noexcept {
        return std::find_if(attributes.cbegin(), attributes.cend(), [](const auto& item) {
            return item->type == ASTAttribute::Flags;
        }) != attributes.cend();
    }

    bool isHexOutputValues() const noexcept {
        return std::find_if(attributes.cbegin(), attributes.cend(), [](const auto& item) {
            return item->type == ASTAttribute::Hex;
        }) != attributes.cend();
    }

    AST_NODE_ACCEPT
};

struct ASTEnumConst : ASTDecl {
    int64_t value{};

    AST_NODE_ACCEPT
};

struct ASTInterface : ASTType {
    std::vector<ASTMethod*> methods;

    AST_NODE_ACCEPT
};

struct ASTMethod : ASTDecl {
    ASTDeclRef* returnTypeRef{};
    std::vector<ASTParameter*> parameters;

    AST_NODE_ACCEPT
};

struct ASTParameter : ASTDecl {
    ASTDeclRef* typeRef{};

    AST_NODE_ACCEPT
};

inline const ASTNamespace* ASTNode::ns() const noexcept {
    auto current = parent;
    while (current && dynamic_cast<const ASTNamespace*>(current) == nullptr) {
        current = current->parent;
    }
    return dynamic_cast<const ASTNamespace*>(current);
}

#endif
