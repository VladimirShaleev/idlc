#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "ast.hpp"
#include "case_converter.hpp"
#include "compilation_result.hpp"
#include "errors.hpp"
#include "options.hpp"

namespace idl {

enum class BoolType {
    Int32,
    Int8,
    StdBool
};

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

    bool useStdTypes() const noexcept {
        return _useStdTypes;
    }

    BoolType boolType() const noexcept {
        return _boolType;
    }

    template <typename Node>
    Node* allocNode(const idl::location& loc) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        _nodes.push_back(nullptr);
        auto node      = new Node{};
        node->location = loc;
        _nodes.back()  = node;
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
        if (decl->as<ASTImport>()) {
            return;
        }
        const auto fullname = decl->fullnameLowecase();
        if (_symbols.contains(fullname)) {
            log<IDL_STATUS_E3012>(decl->location, decl->fullname());
        }
        _symbols[fullname] = decl;
    }

    template <typename Visitor, typename... Args>
    Visitor visit(ASTNode* node, Args&&... args) {
        Visitor visitor(*this, std::forward<Args>(args)...);
        if (node) {
            node->accept(visitor);
        }
        return visitor;
    }

    void initBuiltins() {
        static const std::string filename = "<builtin>";

        const auto loc = idl::location(idl::position(&filename, 1, 1));

        auto addBuiltin =
            [this, &loc]<typename Node>(std::string&& name, const std::string& detail, Node) {
            auto node    = allocNode<Node>(loc);
            node->name   = std::move(name);
            node->parent = _api;
            addSymbol(node);
            _api->decls.push_back(node);
        };

        addBuiltin("Void", "void type.", ASTVoid{});
        addBuiltin("Char", "symbol type.", ASTChar{});
        addBuiltin("Bool", "boolean type.", ASTBool{});
        addBuiltin("Int8", "8 bit signed integer.", ASTInt8{});
        addBuiltin("Uint8", "8 bit unsigned integer.", ASTUint8{});
        addBuiltin("Int16", "16 bit signed integer.", ASTInt16{});
        addBuiltin("Uint16", "16 bit unsigned integer.", ASTUint16{});
        addBuiltin("Int32", "32 bit signed integer.", ASTInt32{});
        addBuiltin("Uint32", "32 bit unsigned integer.", ASTUint32{});
        addBuiltin("Int64", "64 bit signed integer.", ASTInt64{});
        addBuiltin("Uint64", "64 bit unsigned integer.", ASTUint64{});
        addBuiltin("Float32", "32 bit float point.", ASTFloat32{});
        addBuiltin("Float64", "64 bit float point.", ASTFloat64{});
        addBuiltin("Str", "utf8 string.", ASTStr{});
        addBuiltin("Data", "pointer to data.", ASTData{});
    }

    template <idl_status_t Status, typename... Args>
    void log(const idl::location& loc, Args&&... args) {
        if (!_result) {
            return;
        }
        const auto message = err<Status>(loc, std::forward<Args>(args)...);
        _result->addMessage(Status, *loc.begin.filename, loc.begin.line, loc.begin.column, message);
    }

    Options* _options;
    CompilationResult* _result;
    std::vector<idl_message_t> _messages{};
    std::optional<idl_api_version_t> _version{};
    bool _useStdTypes{};
    BoolType _boolType{};
    ASTApi* _api{};
    std::vector<ASTNode*> _nodes{};
    std::unordered_map<std::string, struct ASTDecl*> _symbols{};
    std::unordered_map<std::string, struct ASTDocDecl*> _docSymbols{};
    std::unordered_map<std::string, ASTLiteral*> _literals{};
    uint32_t _lastOrder{};
};

} // namespace idl

#endif
