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

    const ASTApi* api(const idl::location& loc) const noexcept {
        return _api;
    }

    template <typename Node, typename Exception>
    Node* allocNode(const idl::location& loc, int parentToken, int token) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        auto node = new (std::nothrow) Node{};
        if (!node) {
            throw Exception(loc, "out of memory");
        }
        if constexpr (std::is_same<Node, ASTApi>::value) {
            _api = node;
        }
        node->location    = loc;
        node->parentToken = parentToken;
        node->token       = token;
        _nodes.push_back(node);
        return node;
    }

    template <typename Exception>
    ASTLiteral* intern(const idl::location& loc, const std::string& str, int token) {
        const auto key = XXH64(str.c_str(), str.length(), 0);
        if (auto it = _literals.find(key); it != _literals.end()) {
#ifndef NDEBUG
            if (auto strLiteral = dynamic_cast<ASTLiteralStr*>(it->second)) {
                assert(strLiteral->value == str);
            }
#endif
            return it->second;
        }
        auto literal   = allocNode<ASTLiteralStr, Exception>(loc, -1, token);
        literal->value = str;
        _literals[key] = literal;
        return literal;
    }

    void currentDeclLine(int currentDeclLine) noexcept {
        _currentDeclLine = currentDeclLine;
    }

    int currentDeclLine() const noexcept {
        return _currentDeclLine;
    }

    template <typename Node>
    Node* lastType(ASTNode* from) const {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        auto it = std::find(_nodes.rbegin(), _nodes.rend(), from);
        assert(it != _nodes.rend());
        for (++it; it != _nodes.rend(); ++it) {
            if (dynamic_cast<ASTType*>(*it)) {
                if (auto node = dynamic_cast<Node*>(*it)) {
                    return node;
                }
                break;
            }
        }
        return nullptr;
    }

    void calcEnumConsts() {
        filter<ASTEnum>([](auto en) {
            if (en->consts.empty()) {
                err<E2026>(en->location, en->name);
                return false;
            }
            auto attrValue            = en->consts.front()->template findAttr<ASTAttr::Value>();
            int32_t lastValue         = attrValue ? attrValue->args[0].value : 0;
            en->consts.front()->value = lastValue;
            for (size_t i = 1; i < en->consts.size(); ++i) {
                attrValue            = en->consts[i]->template findAttr<ASTAttr::Value>();
                const auto value     = attrValue ? attrValue->args[0].value : lastValue + 1;
                lastValue            = value;
                en->consts[i]->value = value;
            }
            return true;
        });
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
    int _currentDeclLine{ -1 };
};

} // namespace idl

#endif
