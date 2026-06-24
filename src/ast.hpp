#ifndef AST_HPP
#define AST_HPP

#include "idl.hpp"
#include "location.hh"

namespace idl {

struct Visitor;

struct ASTNode {
    virtual ~ASTNode() = default;

    virtual void accept(Visitor& visitor) = 0;

    template <typename Node>
    Node* as() noexcept {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        return dynamic_cast<Node*>(this);
    }

    ASTNode* parent{};
    idl::location location{};
};

struct ASTLiteral : ASTNode {};

struct ASTLiteralBool : ASTLiteral {
    void accept(Visitor& visitor) override;

    bool value;
};

struct ASTLiteralInt : ASTLiteral {
    void accept(Visitor& visitor) override;

    int64_t value;
};

struct ASTLiteralFloat : ASTLiteral {
    void accept(Visitor& visitor) override;

    double value;
};

struct ASTLiteralStr : ASTLiteral {
    void accept(Visitor& visitor) override;

    std::string value;
};

struct ASTAttr : ASTNode {};

struct ASTDecl : ASTNode {
    std::string name;
    uint32_t order;
    std::vector<ASTAttr*> attrs;

    template <typename Attr>
    Attr* findAttr() noexcept {
        static_assert(std::is_base_of<ASTAttr, Attr>::value, "Attr must be inherited from ASTAttr");
        auto it = std::find_if(attrs.begin(), attrs.end(), [](auto attr) {
            return typeid(*attr) == typeid(Attr);
        });
        return it != attrs.end() ? (*it)->template as<Attr>() : nullptr;
    }

    std::string fullname() const;
    std::string fullnameLowecase() const;
};

struct ASTType : ASTDecl {};

struct ASTConst : ASTDecl {
    void accept(Visitor& visitor) override;

    bool evaluated{};
    int32_t value{};
};

struct ASTEnum : ASTType {
    void accept(Visitor& visitor) override;

    std::vector<ASTConst*> consts;
};

struct ASTApi : ASTDecl {
    void accept(Visitor& visitor) override;

    std::vector<ASTDecl*> decls;
};

struct ASTAttrFlags : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrHex : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTDocAttr : ASTAttr {
    std::vector<ASTNode*> message;
};

struct ASTAttrBrief : ASTDocAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrDetail : ASTDocAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrValue : ASTAttr {
    void accept(Visitor& visitor) override;

    std::vector<ASTNode*> values;
};

struct ASTAttrType : ASTAttr {
    void accept(Visitor& visitor) override;

    struct ASTDeclRef* type;
};

// struct ASTAttrStatic : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrCtor : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrThis : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrGet : ASTAttr {
//     void accept(Visitor& visitor) override;
//
//     struct ASTDeclRef* decl{};
// };
//
// struct ASTAttrSet : ASTAttr {
//     void accept(Visitor& visitor) override;
//
//     struct ASTDeclRef* decl{};
// };
//
// struct ASTAttrHandle : ASTAttr {
//     void accept(Visitor& visitor) override;
// };

struct ASTAttrCName : ASTAttr {
    void accept(Visitor& visitor) override;

    std::string name;
};

// struct ASTAttrArray : ASTAttr {
//     void accept(Visitor& visitor) override;
//
//     bool ref{};
//     int size{};
//     struct ASTDeclRef* decl{};
// };
//
// struct ASTAttrDataSize : ASTAttr {
//     void accept(Visitor& visitor) override;
//
//     struct ASTDeclRef* decl{};
// };
//
// struct ASTAttrConst : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrRef : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrRefInc : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrUserData : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrErrorCode : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrNoError : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrResult : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrDestroy : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrIn : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrOut : ASTAttr {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTAttrOptional : ASTAttr {
//     void accept(Visitor& visitor) override;
// };

struct ASTAttrTokenizer : ASTAttr {
    void accept(Visitor& visitor) override;

    std::vector<int> nums;
};

struct ASTAttrOrder : ASTAttr {
    void accept(Visitor& visitor) override;

    bool autoOrder;
};

struct ASTAttrSingle : ASTAttr {
    void accept(Visitor& visitor) override;

    bool singleOutput;
};

struct ASTAttrVersion : ASTAttr {
    void accept(Visitor& visitor) override;

    struct Semver {
        uint8_t major{};
        uint8_t minor{};
        uint8_t micro{};
    };

    std::variant<Semver, std::string> version;
};

// struct ASTYear : ASTDocDecl {
//     void accept(Visitor& visitor) override;
//
//     int value{};
// };
//
// struct ASTMajor : ASTDocDecl {
//     void accept(Visitor& visitor) override;
//
//     int value{};
// };
//
// struct ASTMinor : ASTDocDecl {
//     void accept(Visitor& visitor) override;
//
//     int value{};
// };
//
// struct ASTMicro : ASTDocDecl {
//     void accept(Visitor& visitor) override;
//
//     int value{};
// };
//
// struct ASTDocBool : ASTDocDecl {
//     void accept(Visitor& visitor) override;
//
//     bool value{};
// };
//
struct ASTDeclRef : ASTNode {
    void accept(Visitor& visitor) override;

    std::string name;
    ASTDecl* decl{};
};

struct ASTTrivialType : ASTType {};

struct ASTBuiltinType : ASTTrivialType {};

struct ASTIntegerType : ASTBuiltinType {};

struct ASTFloatType : ASTBuiltinType {};

struct ASTVoid : ASTBuiltinType {
    void accept(Visitor& visitor) override;
};

struct ASTChar : ASTBuiltinType {
    void accept(Visitor& visitor) override;
};

struct ASTStr : ASTBuiltinType {
    void accept(Visitor& visitor) override;
};

struct ASTBool : ASTBuiltinType {
    void accept(Visitor& visitor) override;
};

struct ASTInt8 : ASTIntegerType {
    void accept(Visitor& visitor) override;
};

struct ASTUint8 : ASTIntegerType {
    void accept(Visitor& visitor) override;
};

struct ASTInt16 : ASTIntegerType {
    void accept(Visitor& visitor) override;
};

struct ASTUint16 : ASTIntegerType {
    void accept(Visitor& visitor) override;
};

struct ASTInt32 : ASTIntegerType {
    void accept(Visitor& visitor) override;
};

struct ASTUint32 : ASTIntegerType {
    void accept(Visitor& visitor) override;
};

struct ASTInt64 : ASTIntegerType {
    void accept(Visitor& visitor) override;
};

struct ASTUint64 : ASTIntegerType {
    void accept(Visitor& visitor) override;
};

struct ASTFloat32 : ASTFloatType {
    void accept(Visitor& visitor) override;
};

struct ASTFloat64 : ASTFloatType {
    void accept(Visitor& visitor) override;
};

struct ASTData : ASTBuiltinType {
    void accept(Visitor& visitor) override;
};

// struct ASTEnumConst : ASTDecl {
//     void accept(Visitor& visitor) override;
//
//     bool evaluated{};
//     int32_t value{};
// };
//

//
// struct ASTField : ASTDecl {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTStruct : ASTType {
//     void accept(Visitor& visitor) override;
//
//     std::vector<ASTField*> fields;
// };
//
// struct ASTArg : ASTDecl {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTMethod : ASTDecl {
//     void accept(Visitor& visitor) override;
//
//     std::vector<ASTArg*> args;
// };
//
// struct ASTProperty : ASTDecl {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTEvent : ASTDecl {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTInterface : ASTType {
//     void accept(Visitor& visitor) override;
//
//     std::vector<ASTMethod*> methods;
//     std::vector<ASTProperty*> props;
//     std::vector<ASTEvent*> events;
// };
//
// struct ASTHandle : ASTType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTFunc : ASTDecl {
//     void accept(Visitor& visitor) override;
//
//     std::vector<ASTArg*> args;
// };
//
// struct ASTCallback : ASTType {
//     void accept(Visitor& visitor) override;
//
//     std::vector<ASTArg*> args;
// };
//

struct ASTImport : ASTDecl {
    void accept(Visitor& visitor) override;

    std::vector<ASTDecl*> decls;
};

struct Visitor {
    explicit Visitor(class Context& ctx) noexcept;
    virtual ~Visitor();

    virtual void visit(ASTLiteralStr* node);
    virtual void visit(ASTLiteralBool* node);
    virtual void visit(ASTLiteralInt* node);
    virtual void visit(ASTLiteralFloat* node);
    virtual void visit(ASTApi* node);
    virtual void visit(ASTEnum* node);
    virtual void visit(ASTConst* node);
    virtual void visit(ASTAttrTokenizer* node);
    virtual void visit(ASTAttrOrder* node);
    virtual void visit(ASTAttrSingle* node);
    virtual void visit(ASTAttrVersion* node);
    virtual void visit(ASTAttrFlags* node);
    virtual void visit(ASTAttrHex* node);
    virtual void visit(ASTAttrBrief* node);
    virtual void visit(ASTAttrDetail* node);
    virtual void visit(ASTAttrValue* node);
    virtual void visit(ASTAttrType* node);
    virtual void visit(ASTAttrCName* node);
    virtual void visit(ASTDeclRef* node);
    virtual void visit(ASTImport* node);
    virtual void visit(ASTVoid* node);
    virtual void visit(ASTChar* node);
    virtual void visit(ASTStr* node);
    virtual void visit(ASTBool* node);
    virtual void visit(ASTInt8* node);
    virtual void visit(ASTUint8* node);
    virtual void visit(ASTInt16* node);
    virtual void visit(ASTUint16* node);
    virtual void visit(ASTInt32* node);
    virtual void visit(ASTUint32* node);
    virtual void visit(ASTInt64* node);
    virtual void visit(ASTUint64* node);
    virtual void visit(ASTFloat32* node);
    virtual void visit(ASTFloat64* node);
    virtual void visit(ASTData* node);
    virtual void discarded(ASTNode* node);

    class Context& ctx;
};

} // namespace idl

#endif
