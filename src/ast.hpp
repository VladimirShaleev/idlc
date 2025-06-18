#ifndef AST_HPP
#define AST_HPP

#include <algorithm>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <typeindex>
#include <vector>

#include <magic_enum/magic_enum.hpp>

#include "errors.hpp"
#include "location.hh"

struct Visitor;

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

struct ASTLiteralConsts : ASTLiteral {
    void accept(Visitor& visitor) override;

    std::vector<struct ASTDeclRef*> decls;
};

struct ASTLiteralStr : ASTLiteral {
    void accept(Visitor& visitor) override;

    std::string value;
};

struct ASTDoc : ASTNode {
    void accept(Visitor& visitor) override;

    std::vector<ASTNode*> brief;
    std::vector<ASTNode*> detail;
    std::vector<ASTNode*> ret;
    std::vector<ASTNode*> copyright;
    std::vector<ASTNode*> license;
    std::vector<std::vector<ASTNode*>> authors;
    std::vector<std::vector<ASTNode*>> see;
    std::vector<std::vector<ASTNode*>> note;
    std::vector<std::vector<ASTNode*>> warn;
};

struct ASTAttr : ASTNode {};

struct ASTAttrPlatform : ASTAttr {
    enum Type {
        Windows = 1,
        Linux   = 2,
        MacOS   = 4,
        Web     = 8,
        Android = 16,
        iOS     = 32
    };

    void accept(Visitor& visitor) override;

    Type platforms;
};

struct ASTAttrFlags : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrHex : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrValue : ASTAttr {
    void accept(Visitor& visitor) override;

    ASTLiteral* value;
};

struct ASTAttrType : ASTAttr {
    void accept(Visitor& visitor) override;

    struct ASTDeclRef* type;
};

struct ASTAttrStatic : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrCtor : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrThis : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrGet : ASTAttr {
    void accept(Visitor& visitor) override;

    struct ASTDeclRef* decl{};
};

struct ASTAttrSet : ASTAttr {
    void accept(Visitor& visitor) override;

    struct ASTDeclRef* decl{};
};

struct ASTAttrHandle : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrCName : ASTAttr {
    void accept(Visitor& visitor) override;

    std::string name;
};

struct ASTAttrArray : ASTAttr {
    void accept(Visitor& visitor) override;

    bool ref{};
    int size{};
    struct ASTDeclRef* decl{};
};

struct ASTAttrConst : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrRef : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrUserData : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrErrorCode : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrResult : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTAttrDestroy : ASTAttr {
    void accept(Visitor& visitor) override;
};

struct ASTDecl : ASTNode {
    std::string name;
    std::vector<ASTAttr*> attrs;
    ASTDoc* doc{};

    template <typename Attr>
    Attr* findAttr() noexcept {
        auto it = std::find_if(attrs.begin(), attrs.end(), [](auto attr) {
            return typeid(*attr) == typeid(Attr);
        });
        return it != attrs.end() ? dynamic_cast<Attr*>(*it) : nullptr;
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
    void accept(Visitor& visitor) override;

    std::string name;
    ASTDecl* decl{};
};

struct ASTType : ASTDecl {};

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

struct ASTEnumConst : ASTDecl {
    void accept(Visitor& visitor) override;

    bool evaluated{};
    int32_t value{};
};

struct ASTEnum : ASTType {
    void accept(Visitor& visitor) override;

    std::vector<ASTEnumConst*> consts;
};

struct ASTField : ASTDecl {
    void accept(Visitor& visitor) override;
};

struct ASTStruct : ASTType {
    void accept(Visitor& visitor) override;

    std::vector<ASTField*> fields;
};

struct ASTArg : ASTDecl {
    void accept(Visitor& visitor) override;
};

struct ASTMethod : ASTDecl {
    void accept(Visitor& visitor) override;

    std::vector<ASTArg*> args;
};

struct ASTProperty : ASTDecl {
    void accept(Visitor& visitor) override;
};

struct ASTInterface : ASTType {
    void accept(Visitor& visitor) override;

    std::vector<ASTMethod*> methods;
    std::vector<ASTProperty*> props;
};

struct ASTHandle : ASTType {
    void accept(Visitor& visitor) override;
};

struct ASTFunc : ASTDecl {
    void accept(Visitor& visitor) override;

    std::vector<ASTArg*> args;
};

struct ASTCallback : ASTDecl {
    void accept(Visitor& visitor) override;

    std::vector<ASTArg*> args;
};

struct ASTApi : ASTDecl {
    void accept(Visitor& visitor) override;

    std::vector<ASTEnum*> enums;
    std::vector<ASTStruct*> structs;
    std::vector<ASTCallback*> callbacks;
    std::vector<ASTFunc*> funcs;
    std::vector<ASTInterface*> interfaces;
    std::vector<ASTHandle*> handles;
};

struct Visitor {
    virtual ~Visitor() = default;

    virtual void visit(ASTLiteralStr* node) {
        discarded(node);
    }

    virtual void visit(ASTLiteralBool* node) {
        discarded(node);
    }

    virtual void visit(ASTLiteralInt* node) {
        discarded(node);
    }

    virtual void visit(ASTLiteralConsts* node) {
        discarded(node);
    }

    virtual void visit(ASTDoc* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrPlatform* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrFlags* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrHex* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrValue* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrType* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrStatic* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrCtor* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrThis* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrGet* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrSet* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrCName* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrArray* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrConst* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrRef* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrUserData* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrErrorCode* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrResult* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrDestroy* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrHandle* node) {
        discarded(node);
    }

    virtual void visit(ASTDeclRef* node) {
        discarded(node);
    }

    virtual void visit(ASTVoid* node) {
        discarded(node);
    }

    virtual void visit(ASTChar* node) {
        discarded(node);
    }

    virtual void visit(ASTStr* node) {
        discarded(node);
    }

    virtual void visit(ASTBool* node) {
        discarded(node);
    }

    virtual void visit(ASTInt8* node) {
        discarded(node);
    }

    virtual void visit(ASTUint8* node) {
        discarded(node);
    }

    virtual void visit(ASTInt16* node) {
        discarded(node);
    }

    virtual void visit(ASTUint16* node) {
        discarded(node);
    }

    virtual void visit(ASTInt32* node) {
        discarded(node);
    }

    virtual void visit(ASTUint32* node) {
        discarded(node);
    }

    virtual void visit(ASTInt64* node) {
        discarded(node);
    }

    virtual void visit(ASTUint64* node) {
        discarded(node);
    }

    virtual void visit(ASTFloat32* node) {
        discarded(node);
    }

    virtual void visit(ASTFloat64* node) {
        discarded(node);
    }

    virtual void visit(ASTData* node) {
        discarded(node);
    }

    virtual void visit(ASTApi* node) {
        discarded(node);
    }

    virtual void visit(ASTEnum* node) {
        discarded(node);
    }

    virtual void visit(ASTEnumConst* node) {
        discarded(node);
    }

    virtual void visit(ASTStruct* node) {
        discarded(node);
    }

    virtual void visit(ASTField* node) {
        discarded(node);
    }

    virtual void visit(ASTInterface* node) {
        discarded(node);
    }

    virtual void visit(ASTHandle* node) {
        discarded(node);
    }

    virtual void visit(ASTFunc* node) {
        discarded(node);
    }

    virtual void visit(ASTCallback* node) {
        discarded(node);
    }

    virtual void visit(ASTMethod* node) {
        discarded(node);
    }

    virtual void visit(ASTProperty* node) {
        discarded(node);
    }

    virtual void visit(ASTArg* node) {
        discarded(node);
    }

    virtual void discarded(ASTNode* node) {
    }
};

inline void ASTLiteralStr::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTLiteralBool::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTLiteralInt::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTLiteralConsts::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTDoc::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrPlatform::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrFlags::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrHex::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrValue::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrType::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrStatic::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrCtor::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrThis::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrGet::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrSet::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrCName::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrArray::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrConst::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrRef::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrUserData::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrErrorCode::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrResult::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrDestroy::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrHandle::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTDeclRef::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTVoid::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTChar::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTStr::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTBool::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTInt8::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTUint8::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTInt16::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTUint16::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTInt32::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTUint32::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTInt64::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTUint64::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTFloat32::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTFloat64::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTData::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTApi::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTEnum::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTEnumConst::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTStruct::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTField::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTInterface::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTHandle::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTFunc::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTCallback::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTMethod::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTProperty::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTArg::accept(Visitor& visitor) {
    visitor.visit(this);
}

#endif
