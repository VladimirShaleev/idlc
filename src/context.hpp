#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "ast.hpp"
#include "case_converter.hpp"
#include "compilation_result.hpp"
#include "errors.hpp"
#include "options.hpp"

namespace idl {

class Context final {
public:
    Context(Options* options, CompilationResult* result) noexcept : _options(options), _result(result) {
    }

    ~Context() {
        for (auto node : _nodes) {
            delete node;
        }
    }

    void build() {
        if (!_messages.empty()) {
            return;
        }
        printf("Building AST...\n");
    }

    ASTApi* api() noexcept {
        return _api;
    }

    template <typename Node>
    Node* allocNode(const idl::location& loc) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        _nodes.push_back(nullptr);

        auto node      = new Node{};
        node->location = loc;
        _nodes.back()  = node;

        // if constexpr (std::is_same<Node, ASTApi>::value) {
        //     _api = node;
        // } else if (node->parent) {
        //     ChildAdder adder(node);
        //     node->parent->accept(adder);
        // }
        return node;
    }

    template <typename T>
    ASTLiteral* addLiteral(const idl::location& loc, const T& value) {
        std::string valueStr;
        if constexpr (std::is_same_v<T, std::string>) {
            valueStr = value;
        } else {
            valueStr = std::to_string(value);
        }
        auto hash = std::hash<std::string>{}(valueStr);
        auto key  = std::string(typeid(T).name()) + '|' + std::to_string(hash);

        auto it = _literals.find(key);
        if (it != _literals.end()) {
            return it->second;
        }
        ASTLiteral* node = nullptr;
        if constexpr (std::is_same_v<T, std::string>) {
            if (node = new (std::nothrow) ASTLiteralStr{}) {
                node->as<ASTLiteralStr>()->value = value;
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            if (node = new (std::nothrow) ASTLiteralBool{}) {
                node->as<ASTLiteralBool>()->value = value;
            }
        } else if constexpr (std::is_integral_v<T>) {
            if (node = new (std::nothrow) ASTLiteralInt{}) {
                node->as<ASTLiteralInt>()->value = value;
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if (node = new (std::nothrow) ASTLiteralFloat{}) {
                node->as<ASTLiteralFloat>()->value = value;
            }
        } else {
            assert(!"unsupported literal type");
        }
        if (!node) {
            // err<IDL_STATUS_E2045>(location); // TODO
            return nullptr;
        }
        node->location = loc;
        _literals[key] = node;
        return node;
    }

    void addSymbol(ASTDecl* decl) {
        decl->order = ++_lastOrder;
        if (auto import = decl->as<ASTImport>()) {
            _imports.push_back(import);
            return;
        }
        const auto fullname = decl->fullnameLowecase();
        if (_symbols.contains(fullname)) {
            log<IDL_STATUS_E3012>(decl->location, decl->fullname());
        }
        _symbols[fullname] = decl;
        // if (!_files.empty()) {
        //     decl->file = _files.back();
        //     _files.back()->decls.push_back(decl);
        // }
    }

    template <typename Visitor, typename... Args>
    Visitor visit(ASTNode* node, Args&&... args) {
        Visitor visitor(*this, std::forward<Args>(args)...);
        if (node) {
            node->accept(visitor);
        }
        return visitor;
    }

    void popImport() {
        _imports.pop_back();
        _nodes.push_back(nullptr);
    }

    ASTImport* topImport() const noexcept {
        return _imports.empty() ? nullptr : _imports.back();
    }

    ASTDecl* prevDecl() const noexcept {
        for (auto it = _nodes.rbegin(); it != _nodes.rend(); ++it) {
            if (*it == nullptr) {
                return nullptr;
            }
            if (auto decl = (*it)->as<ASTDecl>()) {
                for (++it; it != _nodes.rend(); ++it) {
                    if (*it == nullptr) {
                        return nullptr;
                    }
                    if (auto decl = (*it)->as<ASTDecl>()) {
                        return decl;
                    }
                }
                return nullptr;
            }
        }
        return nullptr;
    }

    template <idl_status_t Status, typename... Args>
    void log(const idl::location& loc, Args&&... args) {
        if (!_result) {
            return;
        }

        std::string str;
        if constexpr (Status == IDL_STATUS_N1001) {
            str = fmt::format("Unnecessary parentheses for a parameterless attribute '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_W2001) {
            str = fmt::format("The declaration '{}' is missing an attribute [{}]", args...);
        } else if constexpr (Status == IDL_STATUS_W2002) {
            str = fmt::format("Repeated import '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3001) {
            if constexpr (sizeof...(args) > 0) {
                str = fmt::format("Syntax error '{}'", args...);
            } else {
                str = fmt::format("Syntax error");
            }
        } else if constexpr (Status == IDL_STATUS_E3002) {
            str = fmt::format("Argument parsing error '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3003) {
            str = fmt::format("The [version] attribute must have three required integer parameters, such as version(1, "
                              "2, 3) or version(\"string\")");
        } else if constexpr (Status == IDL_STATUS_E3004) {
            str = fmt::format("Version values must be between 0 and 255, while the argument is {}", args...);
        } else if constexpr (Status == IDL_STATUS_E3005) {
            str = fmt::format("Invalid attribute '{}' for {} '{}' declaration, allowed attributes are {}", args...);
        } else if constexpr (Status == IDL_STATUS_E3006) {
            str = fmt::format("Attributes are not allowed for the {} '{}' declaration", args...);
        } else if constexpr (Status == IDL_STATUS_E3007) {
            str = fmt::format("Attribute duplication for attribute '{}' in {} '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3008) {
            str = fmt::format("The attribute '{}' must not have arguments", args...);
        } else if constexpr (Status == IDL_STATUS_E3009) {
            str = fmt::format("String closing character not found in string \"{}\"", args...);
        } else if constexpr (Status == IDL_STATUS_E3010) {
            str = fmt::format("API Redeclaration '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3011) {
            str =
                fmt::format("The first declaration in the description should always begin with the 'api' declaration");
        } else if constexpr (Status == IDL_STATUS_E3012) {
            str = fmt::format("Symbol redefinition '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3013) {
            str = fmt::format("Unknown attribute '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3014) {
            str = fmt::format("The [brief] attribute must contain one or more arguments");
        } else if constexpr (Status == IDL_STATUS_E3015) {
            str = fmt::format("Unknown attribute in the documentation '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3016) {
            str = fmt::format("The documentation string cannot be empty", args...);
        } else if constexpr (Status == IDL_STATUS_E3017) {
            str = fmt::format("The [detail] attribute must contain one or more arguments");
        } else if constexpr (Status == IDL_STATUS_E3018) {
            str = fmt::format("Inline documentation only [detail] description is allowed");
        } else if constexpr (Status == IDL_STATUS_E3019) {
            str = fmt::format("Unexpected character '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3020) {
            str = fmt::format("Tabs are not allowed");
        } else if constexpr (Status == IDL_STATUS_E3021) {
            str = fmt::format("could not find file '{}' for import", args...);
        } else if constexpr (Status == IDL_STATUS_E3022) {
            str = fmt::format("Failed to open file '{}'", args...);
        } else if constexpr (Status == IDL_STATUS_E3023) {
            str = fmt::format("A 'const' '{}' can be defined only for an 'enum'", args...);
        } else {
            assert(!"Unknown status code");
        }

        _result->addMessage(Exception(Status, *loc.begin.filename, loc.begin.line, loc.begin.column, str),
                            Status >= IDL_STATUS_E3001);
    }

    Options* _options;
    CompilationResult* _result;
    std::vector<idl_message_t> _messages{};
    std::optional<idl_api_version_t> _version{};
    ASTApi* _api{};
    std::vector<ASTNode*> _nodes{};
    std::unordered_map<std::string, struct ASTDecl*> _symbols{};
    std::unordered_map<std::string, struct ASTDocDecl*> _docSymbols{};
    std::unordered_map<std::string, ASTLiteral*> _literals{};
    std::vector<ASTImport*> _imports{};
    uint32_t _lastOrder{};
};

} // namespace idl

#endif
