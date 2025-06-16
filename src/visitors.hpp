#ifndef VISITORS_HPP
#define VISITORS_HPP

#include "ast.hpp"
#include "errors.hpp"

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

    void visit(struct ASTAttrThis* node) override {
        str = "this";
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
        allowed = { add<ASTAttrPlatform>() };
    }

    void visit(ASTField* node) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrValue>() };
    }

    void visit(ASTInterface* node) override {
        allowed = { add<ASTAttrPlatform>() };
    }

    void visit(ASTMethod* node) override {
        allowed = { add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrStatic>(), add<ASTAttrCtor>() };
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
