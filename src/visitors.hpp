#ifndef VISITORS_HPP
#define VISITORS_HPP

#include "ast.hpp"
#include "case_converter.hpp"
#include "errors.hpp"

struct CName : Visitor {
    void visit(ASTVoid* node) override {
        str    = "void";
        native = "void";
    }

    void visit(ASTChar* node) override {
        str    = cname(node) + "_t";
        native = "char";
    }

    void visit(ASTStr* node) override {
        ASTStr type{};
        type.parent = node->parent;
        type.name   = "Utf8";
        str         = cname(&type) + "_t";
        native      = "const char*";
    }

    void visit(ASTBool* node) override {
        str    = cname(node) + "_t";
        native = "int32_t";
    }

    void visit(ASTInt8* node) override {
        ASTInt8 type{};
        type.parent = node->parent;
        type.name   = "Sint8";
        str         = cname(&type) + "_t";
        native      = "int8_t";
    }

    void visit(ASTUint8* node) override {
        str    = cname(node) + "_t";
        native = "uint8_t";
    }

    void visit(ASTInt16* node) override {
        ASTInt16 type{};
        type.parent = node->parent;
        type.name   = "Sint18";
        str         = cname(&type) + "_t";
        native      = "int16_t";
    }

    void visit(ASTUint16* node) override {
        str    = cname(node) + "_t";
        native = "uint16_t";
    }

    void visit(ASTInt32* node) override {
        ASTInt32 type{};
        type.parent = node->parent;
        type.name   = "Sint32";
        str         = cname(&type) + "_t";
        native      = "int32_t";
    }

    void visit(ASTUint32* node) override {
        str    = cname(node) + "_t";
        native = "uint32_t";
    }

    void visit(ASTInt64* node) override {
        ASTInt64 type{};
        type.parent = node->parent;
        type.name   = "Sint64";
        str         = cname(&type) + "_t";
        native      = "int64_t";
    }

    void visit(ASTUint64* node) override {
        str    = cname(node) + "_t";
        native = "uint64_t";
    }

    void visit(ASTFloat32* node) override {
        str    = cname(node) + "_t";
        native = "float";
    }

    void visit(ASTFloat64* node) override {
        str    = cname(node) + "_t";
        native = "double";
    }

    void visit(ASTEnum* node) override {
        str = cname(node);
        if (node->findAttr<ASTAttrFlags>()) {
            str += "_flags";
        }
        str += "_t";
    }

    void visit(ASTEnumConst* node) override {
        str = cname(node, true);
        if (node->parent->as<ASTDecl>()->findAttr<ASTAttrFlags>()) {
            str += "_BIT";
        }
    }

    void discarded(ASTNode*) override {
        assert(!"C name is missing");
    }

    static std::string cname(ASTDecl* decl, bool upper = false) {
        auto name = convert(decl->name, upper ? Case::ScreamingSnakeCase : Case::SnakeCase);
        if (auto parentDecl = decl->parent->as<ASTDecl>()) {
            return cname(parentDecl, upper) + '_' + name;
        }
        return name;
    }

    std::string str;
    std::string native;
};

struct AttrName : Visitor {
    void visit(ASTAttrPlatform*) override {
        str = "platform";
    }

    void visit(ASTAttrFlags*) override {
        str = "flags";
    }

    void visit(ASTAttrHex*) override {
        str = "hex";
    }

    void visit(ASTAttrValue*) override {
        str = "value";
    }

    void visit(ASTAttrType*) override {
        str = "type";
    }

    void visit(ASTAttrStatic* node) override {
        str = "static";
    }

    void visit(ASTAttrCtor* node) override {
        str = "ctor";
    }

    void visit(ASTAttrThis* node) override {
        str = "this";
    }

    void visit(ASTAttrGet* node) override {
        str = "get";
    }

    void visit(ASTAttrSet* node) override {
        str = "set";
    }

    void visit(ASTAttrHandle* node) override {
        str = "handle";
    }

    void discarded(ASTNode*) override {
        assert(!"attribute name is missing");
    }

    std::string str;
};

struct AllowedAttrs : Visitor {
    void visit(struct ASTEnum*) override {
        allowed = { add<ASTAttrFlags>(), add<ASTAttrHex>(), add<ASTAttrPlatform>() };
    }

    void visit(struct ASTEnumConst*) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrValue>() };
    }

    void visit(ASTStruct* node) override {
        allowed = { add<ASTAttrPlatform>(), add<ASTAttrHandle>() };
    }

    void visit(ASTField* node) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrValue>() };
    }

    void visit(ASTInterface* node) override {
        allowed = { add<ASTAttrPlatform>() };
    }

    void visit(ASTHandle* node) override {
        allowed = { add<ASTAttrPlatform>(), add<ASTAttrType>() };
    }

    void visit(ASTMethod* node) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrStatic>(), add<ASTAttrCtor>() };
    }

    void visit(ASTProperty* node) override {
        allowed = {
            add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrStatic>(), add<ASTAttrGet>(), add<ASTAttrSet>()
        };
    }

    void visit(ASTArg* node) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrValue>(), add<ASTAttrThis>() };
    }

    template <typename Attr>
    static std::pair<std::type_index, std::string> add() {
        return { typeid(Attr), getName<Attr>() };
    }

    template <typename Attr>
    static std::string getName() {
        AttrName name;
        Attr attr{};
        attr.accept(name);
        return name.str;
    }

    std::map<std::type_index, std::string> allowed;
};

template <typename Exception>
struct ChildsAggregator : Visitor {
    ChildsAggregator(ASTDecl* node) noexcept : prevNode(node) {
    }

    void visit(ASTEnum* node) override {
        if (auto parent = getParent<ASTApi>()) {
            parent->enums.push_back(node);
            node->parent = parent;
        } else {
            assert(!"unreachable code");
        }
    }

    void visit(ASTEnumConst* node) override {
        if (auto parent = getParent<ASTEnum>()) {
            parent->consts.push_back(node);
            node->parent = parent;
        } else {
            throw Exception(node->location, err_str<E2022>());
        }
    }

    void visit(ASTStruct* node) override {
        if (auto parent = getParent<ASTApi>()) {
            parent->structs.push_back(node);
            node->parent = parent;
        } else {
            assert(!"unreachable code");
        }
    }

    void visit(ASTField* node) override {
        if (auto parent = getParent<ASTStruct>()) {
            parent->fields.push_back(node);
            node->parent = parent;
        } else {
            throw Exception(node->location, err_str<E2027>());
        }
    }

    void visit(ASTInterface* node) override {
        if (auto parent = getParent<ASTApi>()) {
            parent->interfaces.push_back(node);
            node->parent = parent;
        } else {
            assert(!"unreachable code");
        }
    }

    void visit(ASTHandle* node) override {
        if (auto parent = getParent<ASTApi>()) {
            parent->handles.push_back(node);
            node->parent = parent;
        } else {
            assert(!"unreachable code");
        }
    }

    void visit(ASTMethod* node) override {
        if (auto parent = getParent<ASTInterface>()) {
            parent->methods.push_back(node);
            node->parent = parent;
        } else {
            throw Exception(node->location, err_str<E2043>());
        }
    }

    void visit(ASTArg* node) override {
        if (auto parent = getParent<ASTMethod>()) {
            parent->args.push_back(node);
            node->parent = parent;
        } else {
            throw Exception(node->location, err_str<E2044>());
        }
    }

    void visit(ASTProperty* node) override {
        if (auto parent = getParent<ASTInterface>()) {
            parent->props.push_back(node);
            node->parent = parent;
        } else {
            throw Exception(node->location, err_str<E2043>());
        }
    }

    template <typename Node>
    Node* getParent() noexcept {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        ASTNode* current = prevNode;
        while (current && !current->is<Node>()) {
            current = current->parent;
        }
        return current->as<Node>();
    }

    ASTDecl* prevNode;
};

#endif
