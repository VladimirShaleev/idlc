#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "ast_ref.hpp"
#include "case_converter.hpp"
#include "errors.hpp"
#include "fixed_stack.hpp"
#include "options.hpp"

namespace idl {

class Context final {
public:
    Context(Options* options, CompilationResultBase* result) noexcept : _options(options), _result(result) {
        assert(result);
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

        assert(node->name.name.handle != 0);
        FixedStack<String, 20> stack;
        auto curr = node;
        while (curr) {
            if (curr.is<IDL_AST_NODE_TYPE_DECL>() && !curr.is<IDL_AST_NODE_TYPE_IMPORT>()) {
                stack.push(curr->name.name);
            }
            curr = curr.parent();
        }

        auto hasPrev = false;
        std::ostringstream ss;
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
            if (hasPrev) {
                ss << '.';
            }
            hasPrev = true;
            ss << _result->getStr(*it);
        }
        auto fullname = ss.str();

        node->name.fullname = _result->intern(std::string_view(fullname.c_str(), fullname.length()));

        lower(fullname);
        auto fullnameLowercase = _result->intern(std::string_view(fullname.c_str(), fullname.length()));
        if (_symbols.contains(fullnameLowercase)) {
            log<IDL_STATUS_E3012>(node->location, node.fullname());
        }
        _symbols[fullnameLowercase] = decl;
    }

    [[nodiscard]] ASTNodeRef findSymbol(ASTNodeRef& decl,
                                        const ASTLocation& loc,
                                        std::string_view name,
                                        bool onlyType) {
        assert(decl.is<IDL_AST_NODE_TYPE_DECL>());
        char buffer[500];
        char buffer2[50];
        auto nameLower    = concat(buffer2, name, "", name.length(), '\0');
        auto symbolFinded = false;
        auto curr         = decl;
        while (curr) {
            auto currFullname = curr.fullname();
            auto fullname     = concat(buffer, currFullname, nameLower, currFullname.length());
            auto fullnameId   = _result->findStr(fullname);
            if (fullnameId) {
                if (auto it = _symbols.find(fullnameId.value()); it != _symbols.end()) {
                    auto symbol   = getNodeRef(it->second);
                    auto actual   = concat(buffer, currFullname, name, 0);
                    auto actualId = _result->findStr(fullname);
                    if (actualId != symbol->name.fullname) {
                        log<IDL_STATUS_E3036>(loc, actual, symbol.fullname());
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
            }
            curr = curr.parent() ? curr.parent() : emptyNodeRef();
            if (curr.is<IDL_AST_NODE_TYPE_IMPORT>()) {
                curr = curr.parent();
            }
            if (!curr.is<IDL_AST_NODE_TYPE_DECL>()) {
                curr = emptyNodeRef();
            }
        }
        auto nameId = _result->findStr({ nameLower.data(), nameLower.length() });
        if (nameId) {
            if (auto it = _symbols.find(nameId.value()); it != _symbols.end()) {
                auto symbol = getNodeRef(it->second);
                if (_result->findStr(name) != symbol->name.fullname) {
                    log<IDL_STATUS_E3036>(loc, name, symbol.fullname());
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
        }
        if (onlyType && symbolFinded) {
            log<IDL_STATUS_E3043>(loc, name);
        } else {
            log<IDL_STATUS_E3037>(loc, name);
        }
        return emptyNodeRef();
    }

    template <idl_ast_node_type_t Type>
    [[nodiscard]] ASTNodeRef getTrivial() noexcept {
        if (auto it = _trivials.find(Type); it != _trivials.end()) {
            return getNodeRef(it->second);
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
            auto node                         = _result->allocNode(loc, Tag<Type>::type);
            _result->getNode(node)->name.name = _result->intern(name);
            _result->getNode(node)->parent    = api.handle();
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
            getNodeRef(node).setEvaulated();
            _trivials[Tag<Type>::type] = node;

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

    [[nodiscard]] const Options* options() const noexcept {
        return _options;
    }

private:
    template <size_t N>
    std::string_view
    concat(char (&buffer)[N], std::string_view str1, std::string_view str2, size_t lower, char delimeter = '.') {
        if (str1.length() + str2.length() + 2 >= N) {
            throw std::bad_alloc();
        }
        memcpy(buffer, str1.data(), str1.length());
        size_t len = str1.length();
        if (delimeter != '\0') {
            buffer[str1.length()] = delimeter;
            memcpy(buffer + str1.length() + 1, str2.data(), str2.length());
            len += 1 + str2.length();
        }
        buffer[len] = '\0';
        std::transform(buffer, buffer + std::min(lower, len), buffer, [](auto c) {
            return std::tolower(c);
        });
        return { buffer, len };
    }

    Options* _options;
    CompilationResultBase* _result;
    std::unordered_set<std::string> _imports{};
    std::unordered_map<String, ASTNodeHandle> _symbols{};
    std::map<idl_ast_node_type_t, ASTNodeHandle> _trivials{};
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

template <idl_ast_node_type_t Type>
inline ASTNodeRef ASTNodeRef::addDeclType() {
    assert(_node);
    assert(is<IDL_AST_NODE_TYPE_DECL>());

    auto type           = _ctx->getTrivial<Type>();
    auto attrTypeHandle = _ctx->result()->allocNode(_node->location, IDL_AST_NODE_TYPE_ATTR_TYPE);
    auto attrType       = _ctx->getNodeRef(attrTypeHandle);
    attrType->parent    = handle();
    attrType->child     = _ctx->result()->allocNode(_node->location, IDL_AST_NODE_TYPE_DECL_REF);

    auto declRef                 = _ctx->getNodeRef(attrType->child);
    declRef->parent              = attrTypeHandle;
    declRef->valueDeclRef.symbol = type->name.fullname;
    declRef->valueDeclRef.handle = type.handle();
    declRef.setEvaulated();

    addChild(attrType);
    return type;
}

} // namespace idl

#endif
