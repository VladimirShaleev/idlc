#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <sstream>
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
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        if constexpr (std::is_same<Node, ASTProgram>::value) {
            if (_program) {
                return _program;
            }
        }
        auto node = new (std::nothrow) Node{};
        if (!node) {
            throw Exception(loc, "out of memory");
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
            std::ostringstream ss;
            ss << " (allowed args: ";
            for (size_t i = 0; i < std::size(TargetPlatform::names); ++i) {
                ss << TargetPlatform::names[i] << (i + 1 < std::size(TargetPlatform::names) ? ',' : ')');
            }
            auto allowedArgs = ss.str();
            attr->type       = ASTAttribute::Platform;
            if (args.empty()) {
                throw Exception(loc, "the 'platform' attribute no arguments" + allowedArgs);
            }
            std::sort(attr->arguments.begin(), attr->arguments.end());
            auto last = std::unique(attr->arguments.begin(), attr->arguments.end());
            if (last != attr->arguments.end()) {
                throw Exception(loc, "the 'platform' attribute cannot have duplicates in arguments");
            }
            for (const auto& arg : attr->arguments) {
                const std::string* str = nullptr;
                bool found       = false;
                for (const auto& name : TargetPlatform::names) {
                    str = &arg;
                    if (arg == name) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw Exception(loc, "the 'platform' has not supported type '" + *str + "'" + allowedArgs);
                }
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
        return filter<ASTDecl>([this](const auto decl) {
            const auto type = decl->typeLowercase();
            if (_symbols.contains(type)) {
                std::cerr << "error: redefinition of declaration '" + decl->type() + "' at " << Location(decl->location)
                          << std::endl;
                return false;
            }
            _symbols[type] = decl;
            return true;
        });
    }

    bool resolveDeclRef(ASTDeclRef* ref) {
        if (ref->decl) {
            return true;
        }
        auto ns = ref->ns();
        while (ns) {
            auto it = _symbols.find(ns->typeLowercase() + '.' + ref->nameLowercase());
            if (it != _symbols.end() && it->second->type() == ns->type() + '.' + ref->name) {
                ref->decl = it->second;
                break;
            }
            ns = ns->ns();
        }
        if (!ref->decl) {
            auto it = _symbols.find(ref->nameLowercase());
            if (it != _symbols.end() && it->second->type() == ref->name) {
                ref->decl = it->second;
            }
        }
        if (!ref->decl) {
            std::cerr << "error: declaration '" + ref->name + "' not found at " << Location(ref->location) << std::endl;
            return false;
        }
        if (dynamic_cast<ASTDecl*>(ref->parent) && dynamic_cast<ASTType*>(ref->decl) == nullptr) {
            std::cerr << "error: declaration '" + ref->name + "' is not a type at " << Location(ref->location)
                      << std::endl;
            return false;
        }
        return true;
    }

    bool resolveDeclRefs() {
        return filter<ASTDeclRef>([this](auto ref) {
            return resolveDeclRef(ref);
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

    ASTProgram* _program{};
    std::vector<ASTNode*> _nodes{};
    std::unordered_map<std::string, struct ASTDecl*> _symbols{};
};

} // namespace idl

#endif
