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
        str    = cname(node) + "_t";
        native = "const char*";
    }

    void visit(ASTBool* node) override {
        str    = cname(node) + "_t";
        native = "int32_t";
    }

    void visit(ASTInt8* node) override {
        str    = cname(node) + "_t";
        native = "int8_t";
    }

    void visit(ASTUint8* node) override {
        str    = cname(node) + "_t";
        native = "uint8_t";
    }

    void visit(ASTInt16* node) override {
        str    = cname(node) + "_t";
        native = "int16_t";
    }

    void visit(ASTUint16* node) override {
        str    = cname(node) + "_t";
        native = "uint16_t";
    }

    void visit(ASTInt32* node) override {
        str    = cname(node) + "_t";
        native = "int32_t";
    }

    void visit(ASTUint32* node) override {
        str    = cname(node) + "_t";
        native = "uint32_t";
    }

    void visit(ASTInt64* node) override {
        str    = cname(node) + "_t";
        native = "int64_t";
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

    void visit(ASTData* node) override {
        str    = cname(node) + "_t";
        native = "void*";
    }

    void visit(ASTConstData* node) override {
        str    = cname(node) + "_t";
        native = "const void*";
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

    void visit(ASTStruct* node) override {
        str = cname(node) + "_t";
    }

    void visit(ASTField* node) override {
        str = convert(node->name, Case::SnakeCase);
    }

    void visit(ASTInterface* node) override {
        str = cname(node) + "_t";
    }

    void visit(ASTHandle* node) override {
        str = cname(node) + "_h";
    }

    void visit(ASTCallback* node) override {
        str = cname(node) + "_t";
    }

    void visit(ASTFunc* node) override {
        str = cname(node);
    }

    void visit(ASTMethod* node) override {
        str = cname(node);
    }

    void visit(ASTArg* node) override {
        if (auto attr = node->findAttr<ASTAttrCName>()) {
            str = attr->name;
        } else {
            str = convert(node->name, Case::SnakeCase);
        }
    }

    void discarded(ASTNode*) override {
        assert(!"C name is missing");
    }

    static std::string cnameDecl(ASTDecl* decl, bool upper) {
        if (auto attr = decl->findAttr<ASTAttrCName>()) {
            return attr->name;
        }
        std::vector<int>* nums = nullptr;
        if (auto attr = decl->findAttr<ASTAttrTokenizer>()) {
            nums = &attr->nums;
        }
        return convert(decl->name, upper ? Case::ScreamingSnakeCase : Case::SnakeCase, nums);
    }

    static std::string cname(ASTDecl* decl, bool upper = false) {
        auto name = cnameDecl(decl, upper);
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

    void visit(ASTAttrCName* node) override {
        str = "cname";
    }

    void visit(ASTAttrArray* node) override {
        str = "array";
    }

    void visit(ASTAttrConst* node) override {
        str = "const";
    }

    void visit(ASTAttrRef* node) override {
        str = "ref";
    }

    void visit(ASTAttrUserData* node) override {
        str = "userdata";
    }

    void visit(ASTAttrErrorCode* node) override {
        str = "errorcode";
    }

    void visit(ASTAttrResult* node) override {
        str = "result";
    }

    void visit(ASTAttrDestroy* node) override {
        str = "destroy";
    }

    void visit(ASTAttrIn* node) override {
        str = "in";
    }

    void visit(ASTAttrOut* node) override {
        str = "out";
    }

    void visit(ASTAttrTokenizer* node) override {
        str = "tokenizer";
    }

    void discarded(ASTNode*) override {
        assert(!"attribute name is missing");
    }

    std::string str;
};

struct AllowedAttrs : Visitor {
    void visit(struct ASTEnum*) override {
        allowed = { add<ASTAttrFlags>(), add<ASTAttrHex>(),       add<ASTAttrPlatform>(),
                    add<ASTAttrCName>(), add<ASTAttrTokenizer>(), add<ASTAttrErrorCode>() };
    }

    void visit(struct ASTEnumConst*) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrValue>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
    }

    void visit(ASTStruct* node) override {
        allowed = { add<ASTAttrPlatform>(), add<ASTAttrHandle>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
    }

    void visit(ASTField* node) override {
        allowed = { add<ASTAttrType>(),  add<ASTAttrValue>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>(),
                    add<ASTAttrArray>(), add<ASTAttrConst>(), add<ASTAttrRef>() };
    }

    void visit(ASTInterface* node) override {
        allowed = { add<ASTAttrPlatform>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
    }

    void visit(ASTHandle* node) override {
        allowed = { add<ASTAttrPlatform>(), add<ASTAttrType>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
    }

    void visit(ASTMethod* node) override {
        allowed = { add<ASTAttrType>(),  add<ASTAttrPlatform>(), add<ASTAttrStatic>(),
                    add<ASTAttrCtor>(),  add<ASTAttrCName>(),    add<ASTAttrTokenizer>(),
                    add<ASTAttrConst>(), add<ASTAttrRef>(),      add<ASTAttrDestroy>() };
    }

    void visit(ASTProperty* node) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrStatic>(),   add<ASTAttrGet>(),
                    add<ASTAttrSet>(),  add<ASTAttrCName>(),    add<ASTAttrTokenizer>() };
    }

    void visit(ASTEvent* node) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrStatic>(),   add<ASTAttrGet>(),
                    add<ASTAttrSet>(),  add<ASTAttrCName>(),    add<ASTAttrTokenizer>() };
    }

    void visit(ASTArg* node) override {
        allowed = { add<ASTAttrType>(),      add<ASTAttrValue>(), add<ASTAttrThis>(), add<ASTAttrCName>(),
                    add<ASTAttrTokenizer>(), add<ASTAttrConst>(), add<ASTAttrRef>(),  add<ASTAttrUserData>(),
                    add<ASTAttrResult>(),    add<ASTAttrIn>(),    add<ASTAttrOut>(),  add<ASTAttrArray>() };
    }

    void visit(ASTFunc* node) override {
        allowed = { add<ASTAttrType>(),
                    add<ASTAttrPlatform>(),
                    add<ASTAttrCName>(),
                    add<ASTAttrTokenizer>(),
                    add<ASTAttrErrorCode>() };
    }

    void visit(ASTCallback* node) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
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

    void visit(ASTFunc* node) override {
        if (auto parent = getParent<ASTApi>()) {
            parent->funcs.push_back(node);
            node->parent = parent;
        } else {
            assert(!"unreachable code");
        }
    }

    void visit(ASTCallback* node) override {
        if (auto parent = getParent<ASTApi>()) {
            parent->callbacks.push_back(node);
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
        auto parent = getParentN<ASTMethod, ASTFunc, ASTCallback>();
        if (auto method = parent->template as<ASTMethod>()) {
            method->args.push_back(node);
            node->parent = method;
        } else if (auto func = parent->template as<ASTFunc>()) {
            func->args.push_back(node);
            node->parent = func;
        } else if (auto callback = parent->template as<ASTCallback>()) {
            callback->args.push_back(node);
            node->parent = callback;
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

    void visit(ASTEvent* node) override {
        if (auto parent = getParent<ASTInterface>()) {
            parent->events.push_back(node);
            node->parent = parent;
        } else {
            throw Exception(node->location, err_str<E2090>());
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

    template <typename... Nodes>
    ASTNode* getParentN() noexcept {
        static_assert(((std::is_base_of<ASTNode, Nodes>::value) && ...), "Nodes must be inherited from ASTNode");
        ASTNode* current = prevNode;
        while (current && (!current->is<Nodes>() && ...)) {
            current = current->parent;
        }
        return current;
    }

    ASTDecl* prevNode;
};

#endif
