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

    std::string fullname() const {
        assert(name.length() > 0);
        std::ostringstream ss;
        if (parent) {
            if (auto parentDecl = parent->as<ASTDecl>()) {
                ss << parentDecl->fullname() << '.';
            }
        }
        ss << name;
        return ss.str();
    }

    std::string fullnameLowecase() const {
        auto str = fullname();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        return str;
    }
};

struct ASTType : ASTDecl {};

struct ASTConst : ASTDecl {
    void accept(Visitor& visitor) override;

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

// struct ASTAttrPlatform : ASTAttr {
//     enum Type {
//         Windows = 1,
//         Linux   = 2,
//         MacOS   = 4,
//         Web     = 8,
//         Android = 16,
//         iOS     = 32
//     };
//
//     void accept(Visitor& visitor) override;
//
//     Type platforms;
// };

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

// struct ASTAttrValue : ASTAttr {
//     void accept(Visitor& visitor) override;
//
//     ASTLiteral* value;
// };
//
// struct ASTAttrType : ASTAttr {
//     void accept(Visitor& visitor) override;
//
//     struct ASTDeclRef* type;
// };
//
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
//
// struct ASTAttrCName : ASTAttr {
//     void accept(Visitor& visitor) override;
//
//     std::string name;
// };
//
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
//
// struct ASTAttrTokenizer : ASTAttr {
//     void accept(Visitor& visitor) override;
//
//     std::vector<int> nums;
// };
//
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

// struct ASTTrivialType : ASTType {};
//
// struct ASTBuiltinType : ASTTrivialType {};
//
// struct ASTIntegerType : ASTBuiltinType {};
//
// struct ASTFloatType : ASTBuiltinType {};
//
// struct ASTVoid : ASTBuiltinType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTChar : ASTBuiltinType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTStr : ASTBuiltinType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTBool : ASTBuiltinType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTInt8 : ASTIntegerType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTUint8 : ASTIntegerType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTInt16 : ASTIntegerType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTUint16 : ASTIntegerType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTInt32 : ASTIntegerType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTUint32 : ASTIntegerType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTInt64 : ASTIntegerType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTUint64 : ASTIntegerType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTFloat32 : ASTFloatType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTFloat64 : ASTFloatType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTData : ASTBuiltinType {
//     void accept(Visitor& visitor) override;
// };
//
// struct ASTConstData : ASTBuiltinType {
//     void accept(Visitor& visitor) override;
// };
//
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
    // std::vector<ASTImport*> imports;
    // std::vector<ASTEnum*> enums;
    // std::vector<ASTStruct*> structs;
    // std::vector<ASTCallback*> callbacks;
    // std::vector<ASTFunc*> funcs;
    // std::vector<ASTInterface*> interfaces;
    // std::vector<ASTHandle*> handles;
};

struct Visitor {
    explicit Visitor(class Context& ctx) noexcept : ctx(ctx) {
    }

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

    virtual void visit(ASTLiteralFloat* node) {
        discarded(node);
    }

    virtual void visit(ASTApi* node) {
        discarded(node);
    }

    virtual void visit(ASTEnum* node) {
        discarded(node);
    }

    virtual void visit(ASTConst* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrVersion* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrFlags* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrHex* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrBrief* node) {
        discarded(node);
    }

    virtual void visit(ASTAttrDetail* node) {
        discarded(node);
    }

    virtual void visit(ASTDeclRef* node) {
        discarded(node);
    }

    virtual void visit(ASTImport* node) {
        discarded(node);
    }

    virtual void discarded(ASTNode* node) {
        assert(!"visit method not implemented for this node type");
    }

    class Context& ctx;
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

inline void ASTLiteralFloat::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTApi::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTEnum::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTConst::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrVersion::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrFlags::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrHex::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrBrief::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTAttrDetail::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTDeclRef::accept(Visitor& visitor) {
    visitor.visit(this);
}

inline void ASTImport::accept(Visitor& visitor) {
    visitor.visit(this);
}

} // namespace idl

#endif
