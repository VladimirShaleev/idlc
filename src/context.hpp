#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "ast_ref.hpp"
#include "case_converter.hpp"
#include "errors.hpp"
#include "options.hpp"

namespace idl {

class Context final {
public:
    Context(Options* options, CompilationResultBase* result) noexcept : _options(options), _result(result) {
        assert(result);
    }

    [[nodiscard]] bool useStdTypes() const noexcept {
        return _options ? _options->getCOptions().use_std_types : false;
    }

    [[nodiscard]] bool addDocGroup() const noexcept {
        return _options ? _options->getCOptions().add_doc_group : false;
    }

    [[nodiscard]] idl_bool_type_t boolType() const noexcept {
        return _options ? _options->getBoolType() : IDL_BOOL_TYPE_INT_32;
    }

    [[nodiscard]] bool warnAsErrors() const noexcept {
        return _options ? _options->getWarningsAsErrors() : false;
    }

    [[nodiscard]] ASTNodeRef getNodeRef(ASTNodeHandle handle) noexcept {
        return ASTNodeRef(*this, handle);
    }

    [[nodiscard]] ASTNodeRef emptyNodeRef() noexcept {
        return ASTNodeRef(*this);
    }

    void addChild(ASTNodeHandle parent, ASTNodeHandle child) noexcept {
        getNodeRef(parent).addChild(getNodeRef(child));
    }

    void addSymbol(ASTNodeHandle decl) {
        std::queue<std::pair<ASTNodeRef, ASTNodeRef>> queue;
        queue.emplace(emptyNodeRef(), getNodeRef(decl));
        while (!queue.empty()) {
            auto [parent, curr] = queue.front();
            queue.pop();
            if (parent) {
                curr->parent = parent.handle();
            }
            for (auto child : curr) {
                queue.emplace(curr, child);
            }
        }

        auto node = getNodeRef(decl);
        if (node.is<IDL_AST_NODE_TYPE_IMPORT>()) {
            auto name = node.name();
            std::string nameStr{ name.data(), name.length() };
            addImport(nameStr);
            return;
        }
        const auto fullname = node.fullnameLowercase();
        if (_symbols.contains(fullname)) {
            log<IDL_STATUS_E3012>(node->location, node.fullname());
        }
        _symbols[fullname] = decl;
    }

    [[nodiscard]] auto findSymbol(ASTNodeRef& decl, const ASTLocation& loc, const std::string& name, bool onlyType) {
        auto nameLower = name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](auto c) {
            return std::tolower(c);
        });
        auto symbolFinded = false;
        auto curr         = decl;
        while (curr) {
            const auto fullname = curr.fullnameLowercase() + '.' + nameLower;
            if (auto it = _symbols.find(fullname); it != _symbols.end()) {
                auto symbol = getNodeRef(it->second);

                const auto actualName   = curr.fullname() + '.' + name;
                const auto expectedName = symbol.fullname();
                if (actualName != expectedName) {
                    log<IDL_STATUS_E3036>(loc, actualName, expectedName);
                    return emptyNodeRef();
                }
                symbolFinded = true;
                if (onlyType) {
                    if (symbol.is<IDL_AST_NODE_TYPE_TYPE>()) {
                        return symbol;
                    }
                } else {
                    return symbol;
                }
            }
            curr = curr.parent() ? curr.parent() : emptyNodeRef();
            if (!curr.is<IDL_AST_NODE_TYPE_DECL>()) {
                curr = emptyNodeRef();
            }
        }
        if (auto it = _symbols.find(nameLower); it != _symbols.end()) {
            auto symbol = getNodeRef(it->second);

            const auto expectedName = symbol.fullname();
            if (name != expectedName) {
                log<IDL_STATUS_E3036>(loc, name, expectedName);
                return emptyNodeRef();
            }
            symbolFinded = true;
            if (onlyType) {
                if (symbol.is<IDL_AST_NODE_TYPE_TYPE>()) {
                    return symbol;
                }
            } else {
                return symbol;
            }
        }
        if (onlyType && symbolFinded) {
            log<IDL_STATUS_E3043>(loc, name);
        } else {
            log<IDL_STATUS_E3037>(loc, name);
        }
        return emptyNodeRef();
    }

    void addImport(std::string name) {
        _imports.insert(lower(name));
    }

    [[nodiscard]] bool findImport(ASTNodeRef& decl) const noexcept {
        auto name = decl.name();
        std::string nameStr{ name.data(), name.length() };
        return _imports.contains(lower(nameStr));
    }

    template <typename Visitor, typename... Args>
    Visitor visit(ASTNodeHandle node, Args&&... args) {
        auto nodeRef = getNodeRef(node);
        return nodeRef.template accept<Visitor>(std::forward<Args>(args)...);
    }

    void initBuiltins(ASTNodeRef node) {
        _result->setApi(node.handle());

        const auto loc     = ASTLocation{ _result->intern("<builtin>"), 1, 1 };
        ASTNodeHandle last = HandleNone;

        auto addBuiltin = [this, api = node, &loc, &last]<ASTNodeType Type>(
                              std::string_view name, const std::string& detail, Tag<Type>) mutable {
            auto node                        = _result->allocNode(loc, Tag<Type>::type);
            _result->getNode(node)->valueStr = _result->intern(name);
            _result->getNode(node)->parent   = api.handle();
            addSymbol(node);

            if (last == HandleNone) {
                if (api->child == HandleNone) {
                    api->child = node;
                } else {
                    auto lastChild = HandleNone;
                    auto currChild = api->child;
                    while (currChild != HandleNone) {
                        auto nodeChild = _result->getNode(currChild);
                        lastChild      = currChild;
                        currChild      = nodeChild->sibling;
                    }
                    _result->getNode(lastChild)->sibling = node;
                }
            } else {
                _result->getNode(last)->sibling = node;
            }
            last = node;
        };

        addBuiltin("Void", "void type.", Tag<IDL_AST_NODE_TYPE_VOID>{});
        addBuiltin("Char", "symbol type.", Tag<IDL_AST_NODE_TYPE_CHAR>{});
        addBuiltin("Bool", "boolean type.", Tag<IDL_AST_NODE_TYPE_BOOL>{});
        addBuiltin("Int8", "8 bit signed integer.", Tag<IDL_AST_NODE_TYPE_INT_8>{});
        addBuiltin("Uint8", "8 bit unsigned integer.", Tag<IDL_AST_NODE_TYPE_UINT_8>{});
        addBuiltin("Int16", "16 bit signed integer.", Tag<IDL_AST_NODE_TYPE_INT_16>{});
        addBuiltin("Uint16", "16 bit unsigned integer.", Tag<IDL_AST_NODE_TYPE_UINT_16>{});
        addBuiltin("Int32", "32 bit signed integer.", Tag<IDL_AST_NODE_TYPE_INT_32>{});
        addBuiltin("Uint32", "32 bit unsigned integer.", Tag<IDL_AST_NODE_TYPE_UINT_32>{});
        addBuiltin("Int64", "64 bit signed integer.", Tag<IDL_AST_NODE_TYPE_INT_64>{});
        addBuiltin("Uint64", "64 bit unsigned integer.", Tag<IDL_AST_NODE_TYPE_UINT_64>{});
        addBuiltin("Float32", "32 bit float point.", Tag<IDL_AST_NODE_TYPE_FLOAT_32>{});
        addBuiltin("Float64", "64 bit float point.", Tag<IDL_AST_NODE_TYPE_FLOAT_64>{});
        addBuiltin("Str", "utf8 string.", Tag<IDL_AST_NODE_TYPE_STR>{});
        addBuiltin("Data", "pointer to data.", Tag<IDL_AST_NODE_TYPE_DATA>{});
    }

    template <idl_status_t Status, typename... Args>
    void log(const ASTLocation& loc, Args&&... args) {
        const auto warnAsError = _options ? _options->getWarningsAsErrors() : false;
        _result->addMessage(Status, warnAsError, loc.filename, loc.line, loc.column, [&]() {
            return err<Status>(std::forward<Args>(args)...);
        });
    }

    [[nodiscard]] CompilationResultBase* result() noexcept {
        return _result;
    }

private:
    Options* _options;
    CompilationResultBase* _result;
    std::vector<idl_message_t> _messages{};
    std::optional<idl_api_version_t> _version{};
    std::unordered_set<std::string> _imports{};
    std::unordered_map<std::string, ASTNodeHandle> _symbols{};
    std::unordered_map<std::string, ASTNodeHandle> _docSymbols{};
    std::unordered_map<std::string, ASTNodeHandle> _literals{};
};

inline CompilationResultBase* ASTNodeRef::result() noexcept {
    return _ctx->result();
}

inline const CompilationResultBase* ASTNodeRef::result() const noexcept {
    return _ctx->result();
}

inline ASTNodeRef ASTNodeRef::resolveRef(bool onlyType) {
    if (!evaulated()) {
        setEvaulated();
        auto parentNode = parent();
        while (parentNode) {
            if (parentNode.is<IDL_AST_NODE_TYPE_DECL>()) {
                break;
            }
            parentNode = parentNode.parent();
        }
        auto view = result()->getStr((*this)->valueDeclRef.symbol);
        std::string name(view.data(), view.length());
        if (auto symbol = _ctx->findSymbol(parentNode, (*this)->location, name, onlyType)) {
            (*this)->valueDeclRef.handle = symbol.handle();
            return symbol;
        } else {
            (*this)->valueDeclRef.handle = HandleNone;
        }
    }
    return ASTNodeRef(*_ctx, (*this)->valueDeclRef.handle);
}

} // namespace idl

#endif
