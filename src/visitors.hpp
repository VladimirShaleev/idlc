#ifndef VISITORS_HPP
#define VISITORS_HPP

#include "ast.hpp"

struct HierarchyState {
    enum State {
        Begin,
        End
    };

    State state;
    bool hasPrev;
    bool hasNext;
};

struct HierarchyVisitor : Visitor {
    explicit HierarchyVisitor(Visitor& v) noexcept : visitor(v), state(dynamic_cast<HierarchyState*>(&v)) {
    }

    bool visit(const ASTDeclRef* node) override {
        visitNode(node);
        return true;
    }

    bool visit(const ASTAttribute* node) override {
        visitNode(node);
        return true;
    }

    bool visit(const ASTProgram* node) override {
        visitNode(node, node->namespaces);
        return true;
    }

    bool visit(const ASTNamespace* node) override {
        visitNode(node, node->declarations);
        return true;
    }

    bool visit(const ASTEnum* node) override {
        visitNode(node, node->constants);
        return true;
    }

    bool visit(const ASTEnumConst* node) override {
        visitNode(node);
        return true;
    }

    bool visit(const ASTInterface* node) override {
        visitNode(node, node->methods);
        return true;
    }

    bool visit(const ASTMethod* node) override {
        std::vector ret{ node->returnTypeRef };
        visitNode(node, ret, node->parameters);
        return true;
    }

    bool visit(const ASTParameter* node) override {
        visitNode(node);
        return true;
    }

    template <typename... Childs>
    void visitNode(const ASTNode* node, const Childs&... childs) {
        if (state) {
            state->state = HierarchyState::Begin;
        }
        node->accept(visitor);
        if (auto decl = dynamic_cast<const ASTDecl*>(node)) {
            visitChilds(decl->attributes, [this](auto item, auto hasPrev, auto hasNext) {
                if (state) {
                    state->hasPrev = hasPrev;
                    state->hasNext = hasNext;
                }
                return item->accept(*this);
            });
        }
        (visitChilds(childs,
                     [this](auto item, auto hasPrev, auto hasNext) {
            if (state) {
                state->hasPrev = hasPrev;
                state->hasNext = hasNext;
            }
            return item->accept(*this);
        }),
         ...);
        if (state) {
            state->state = HierarchyState::End;
            node->accept(visitor);
        }
    }

    template <typename Node, typename Func>
    bool visitChilds(const std::vector<Node*>& nodes, Func&& func) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        auto hasPrev = false;
        auto hasNext = true;
        for (size_t i = 0; i < nodes.size(); ++i) {
            hasNext = i + 1 < nodes.size();
            if (!func(nodes[i], hasPrev, hasNext)) {
                return false;
            }
            hasPrev = true;
        }
        return true;
    }

    Visitor& visitor;
    HierarchyState* state;
};

#endif
