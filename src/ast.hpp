#ifndef AST_HPP
#define AST_HPP

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <magic_enum/magic_enum.hpp>

#include "errors.hpp"
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

    virtual void visit(struct ASTLiteralBool* node) {
    }

    virtual void visit(struct ASTLiteralInt* node) {
    }

    virtual void visit(struct ASTLiteralEnumConst* node) {
    }

    virtual void visit(struct ASTDoc* node) {
    }

    virtual void visit(struct ASTAttr* node) {
    }

    virtual void visit(struct ASTDeclRef* node) {
    }

    virtual void visit(struct ASTChar* node) {
    }

    virtual void visit(struct ASTStr* node) {
    }

    virtual void visit(struct ASTBool* node) {
    }

    virtual void visit(struct ASTInt8* node) {
    }

    virtual void visit(struct ASTUint8* node) {
    }

    virtual void visit(struct ASTInt16* node) {
    }

    virtual void visit(struct ASTUint16* node) {
    }

    virtual void visit(struct ASTInt32* node) {
    }

    virtual void visit(struct ASTUint32* node) {
    }

    virtual void visit(struct ASTInt64* node) {
    }

    virtual void visit(struct ASTUint64* node) {
    }

    virtual void visit(struct ASTFloat32* node) {
    }

    virtual void visit(struct ASTFloat64* node) {
    }

    virtual void visit(struct ASTApi* node) {
    }

    virtual void visit(struct ASTEnum* node) {
    }

    virtual void visit(struct ASTEnumConst* node) {
    }

    virtual void visit(struct ASTStruct* node) {
    }

    virtual void visit(struct ASTField* node) {
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
    int token{};
};

struct ASTLiteral : ASTNode {};

struct ASTLiteralBool : ASTLiteral {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    bool value;
};

struct ASTLiteralInt : ASTLiteral {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    int64_t value;
};

struct ASTLiteralEnumConst : ASTLiteral {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::string name;
    struct ASTEnumConst* value{};
};

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
    enum AttrType {
        Platform,
        Flags,
        Hex,
        Value,
        Type
    };

    struct Arg {
        union {
            TargetPlatfrom platform;
            ASTLiteral* value;
            struct ASTDeclRef* type;
        };
    };

    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    AttrType type;
    std::vector<Arg> args;

    static std::string typeStr(AttrType type) {
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

    template <ASTAttr::AttrType Type>
    ASTAttr* findAttr() noexcept {
        auto it = std::find_if(attrs.begin(), attrs.end(), [](auto attr) {
            return attr->type == Type;
        });
        return it != attrs.end() ? *it : nullptr;
    }

    std::string fullname() const {
        assert(name.length() > 0);
        std::string str{};
        if (auto parentDecl = dynamic_cast<ASTDecl*>(parent)) {
            str = parentDecl->fullname() + '.';
        }
        return str + name;
    }

    std::string fullnameLowecase() const {
        auto str = fullname();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        return str;
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

struct ASTTrivialType : ASTType {};

struct ASTBuiltinType : ASTTrivialType {};

struct ASTChar : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTStr : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTBool : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTInt8 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTUint8 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTInt16 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTUint16 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTInt32 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTUint32 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTInt64 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTUint64 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTFloat32 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTFloat64 : ASTBuiltinType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTEnumConst : ASTDecl {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    bool evaluated{};
    int32_t value{};
};

struct ASTEnum : ASTType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::vector<ASTEnumConst*> consts;
};

struct ASTField : ASTDecl {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }
};

struct ASTStruct : ASTType {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::vector<ASTField*> fields;
};

struct ASTApi : ASTDecl {
    void accept(Visitor& visitor) override {
        visitor.visit(this);
    }

    std::vector<ASTEnum*> enums;
    std::vector<ASTStruct*> structs;
};

#endif
