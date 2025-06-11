#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <sstream>
#include <unordered_map>

#include "ast.hpp"
#include "errors.hpp"
#include "location.hpp"

namespace idl {

class Context final {
public:
    ~Context() {
        for (auto node : _nodes) {
            delete node;
        }
    }

    template <typename Node, typename Exception>
    Node* allocNode(const idl::location& loc) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        if constexpr (std::is_same<Node, ASTApi>::value) {
            if (_api) {
                throw Exception(loc, err_str<E2004>());
            }
        }
        auto node = new (std::nothrow) Node{};
        if (!node) {
            throw Exception(loc, "out of memory");
        }
        if constexpr (std::is_same<Node, ASTApi>::value) {
            _api = node;
        }
        node->location = loc;
        _nodes.push_back(node);
        return node;
    }

private:
    template <typename Node, typename Pred>
    bool filter(Pred&& pred) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        for (auto node : _nodes) {
            if (auto ptr = dynamic_cast<Node*>(node)) {
                if (!pred(ptr)) {
                    return false;
                }
            }
        }
        return true;
    }

    ASTApi* _api{};
    std::vector<ASTNode*> _nodes{};
    std::unordered_map<std::string, struct ASTDecl*> _symbols{};
};

} // namespace idl

#endif
