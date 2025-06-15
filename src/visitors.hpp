#ifndef VISITORS_HPP
#define VISITORS_HPP

#include "ast.hpp"
#include "errors.hpp"

struct AllowedAttrs : Visitor {
    void visit(struct ASTEnum*) override {
        allowed = { ASTAttr::Flags, ASTAttr::Hex, ASTAttr::Platform };
    }

    void visit(struct ASTEnumConst*) override {
        allowed = { ASTAttr::Type, ASTAttr::Value };
    }

    void visit(ASTStruct* node) override {
        allowed = { ASTAttr::Platform };
    }

    void visit(ASTField* node) override {
        allowed = { ASTAttr::Type, ASTAttr::Value };
    }

    std::set<ASTAttr::AttrType> allowed;
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
