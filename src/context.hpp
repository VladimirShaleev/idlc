#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <sstream>
#include <unordered_map>
#include <xxhash.h>

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

    template <typename Exception>
    ASTApi* api(const idl::location& loc) {
        if (!_api) {
            throw Exception(loc, err_str<E2012>());
        }
        return _api;
    }

    template <typename Node, typename Exception>
    Node* allocNode(const idl::location& loc) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
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

    template <typename Exception>
    ASTLiteral* intern(const idl::location& loc, const std::string& str) {
        const auto key = XXH64(str.c_str(), str.length(), 0);
        if (auto it = _literals.find(key); it != _literals.end()) {
#ifndef NDEBUG
            if (auto strLiteral = dynamic_cast<ASTLiteralStr*>(it->second)) {
                assert(strLiteral->value == str);
            }
#endif
            return it->second;
        }
        auto literal   = allocNode<ASTLiteralStr, Exception>(loc);
        literal->value = str;
        _literals[key] = literal;
        return literal;
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
    std::unordered_map<uint64_t, ASTLiteral*> _literals{};
};

} // namespace idl

#endif
