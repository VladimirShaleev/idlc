#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <unordered_map>

#include "ast.hpp"
#include "location.hpp"

namespace idl {

class Context final {
public:
    ~Context() {
        for (auto node : _nodes) {
            delete node;
        }
    }

    const ASTProgram* program() const noexcept {
        return _program;
    }

    template <typename Node, typename Exception>
    Node* allocNode(const idl::location& loc) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "T must be inherited from ASTNode");
        if constexpr (std::is_same<Node, ASTProgram>::value) {
            if (_program) {
                return _program;
            }
        }
        auto node = new (std::nothrow) Node{};
        if (!node) {
            throw Exception(loc, "out of memory");
        }
        if constexpr (std::is_base_of<ASTDecl, Node>::value) {
            _decls.push_back(node);
        }
        if constexpr (std::is_same<Node, ASTProgram>::value) {
            _program = node;
        }
        node->location = loc;
        _nodes.push_back(node);
        return node;
    }

    template <typename Exception>
    ASTAttribute* allocAttribute(const idl::location& loc,
                                 const std::string& name,
                                 const std::vector<std::string>& args) {
        auto attr       = allocNode<ASTAttribute, Exception>(loc);
        attr->arguments = args;
        if (name == "flags") {
            attr->type = ASTAttribute::Flags;
            if (!args.empty()) {
                throw Exception(loc, "the 'flags' attribute has no arguments");
            }
        } else if (name == "platform") {
            attr->type = ASTAttribute::Platform;
            if (args.empty()) {
                throw Exception(
                    loc, "the 'platform' attribute no arguments (allowed args: window,linux,macos,web,android,ios)");
            }
            std::sort(attr->arguments.begin(), attr->arguments.end());
            auto last = std::unique(attr->arguments.begin(), attr->arguments.end());
            if (last != attr->arguments.end()) {
                throw Exception(
                    loc, "the 'platform' attribute cannot have duplicates in arguments");
            }
        } else if (name == "hex") {
            attr->type = ASTAttribute::Hex;
            if (!args.empty()) {
                throw Exception(loc, "the 'hex' attribute has no arguments");
            }
        } else if (name == "in") {
            attr->type = ASTAttribute::In;
            if (!args.empty()) {
                throw Exception(loc, "the 'in' attribute has no arguments");
            }
        } else if (name == "out") {
            attr->type = ASTAttribute::Out;
            if (!args.empty()) {
                throw Exception(loc, "the 'out' attribute has no arguments");
            }
        } else {
            throw Exception(loc, "unknown attribute '" + name + '\'');
        }
        return attr;
    }

    bool updateSymbols() {
        for (const auto decl : _decls) {
            const auto type = decl->typeLowercase();
            if (_symbols.contains(type)) {
                std::cerr << "error: redefinition of declaration '" + decl->type() + "' at " << Location(decl->location)
                          << std::endl;
                return false;
            }
            _symbols[type] = decl;
        }
        return true;
    }

private:
    ASTProgram* _program{};
    std::vector<ASTNode*> _nodes{};
    std::vector<ASTDecl*> _decls{};
    std::unordered_map<std::string, struct ASTDecl*> _symbols{};
};

} // namespace idl

#endif
