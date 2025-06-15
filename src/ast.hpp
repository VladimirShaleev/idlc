#ifndef AST_HPP
#define AST_HPP

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <magic_enum/magic_enum.hpp>

#include "location.hh"

enum struct TargetPlatfrom {
    Windows,
    Linux,
    MacOS,
    Web,
    Android,
    iOS
};

struct Visitor {
    virtual ~Visitor() = default;

    virtual void visit(struct ASTLiteralStr* node) {
    }

    virtual void visit(struct ASTDoc* node) {
    }

    virtual void visit(struct ASTAttr* node) {
    }

    virtual void visit(struct ASTDeclRef* node) {
    }

    virtual void visit(struct ASTApi* node) {
    }

    virtual void visit(struct ASTEnum* node) {
    }

    virtual void visit(struct ASTEnumConst* node) {
    }
};

struct ASTNode {
    virtual ~ASTNode() = default;

    virtual void accept(Visitor& visitor) = 0;

    template <typename Node>
    bool is() noexcept {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        return dynamic_cast<Node*>(this) != nullptr;
    }

    template <typename Node>
    Node* as() noexcept {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        return dynamic_cast<Node*>(this);
    }

    ASTNode* parent{};
    idl::location location{};
    int parentToken{};
    int token{};
};

struct ASTLiteral : ASTNode {};

struct ASTLiteralStr : ASTLiteral {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::string value;
};

struct ASTDoc : ASTNode {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::vector<ASTNode*> brief;
    std::vector<ASTNode*> detail;
    std::vector<ASTNode*> copyright;
    std::vector<ASTNode*> license;
    std::vector<std::vector<ASTNode*>> authors;
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

    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

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
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::string name;
    ASTDecl* decl{};
};

struct ASTType : ASTDecl {};

struct ASTEnumConst : ASTDecl {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    int32_t value{};
};

struct ASTEnum : ASTType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::vector<ASTEnumConst*> consts;
};

struct ASTApi : ASTDecl {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::vector<ASTEnum*> enums;
};

#endif
