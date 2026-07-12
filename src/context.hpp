#ifndef CONTEXT_HPP
#define CONTEXT_HPP

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

class ASTNodeRef;

class Context final {
public:
    Context(Options* options, CompilationResultBase* result) noexcept : _options(options), _result(result) {
        assert(result);
    }

    bool useStdTypes() const noexcept {
        return _useStdTypes;
    }

    BoolType boolType() const noexcept {
        return _boolType;
    }

    [[nodiscard]] bool warnAsErrors() const noexcept {
        return _options ? _options->getWarningsAsErrors() : false;
    }

    auto getNodeRef(ASTNodeHandle handle) noexcept;

    void addChild(ASTNodeHandle parent, ASTNodeHandle child) noexcept;

    void addSymbol(ASTNodeHandle decl);

    auto findSymbol(ASTNodeRef& decl, const ASTLocation& loc, const std::string& name, bool onlyType = false);

    template <typename Visitor, typename... Args>
    Visitor visit(ASTNodeHandle node, Args&&... args) {
        auto nodeRef = ASTNodeRef(*this, node);
        return nodeRef.template accept<Visitor>(std::forward<Args>(args)...);
    }

    void initBuiltins(ASTNodeRef node);

    template <idl_status_t Status, typename... Args>
    void log(const ASTLocation& loc, Args&&... args) {
        if (!_result) {
            return;
        }
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
    bool _useStdTypes{};
    BoolType _boolType{};
    std::unordered_map<std::string, ASTNodeHandle> _symbols{};
    std::unordered_map<std::string, ASTNodeHandle> _docSymbols{};
    std::unordered_map<std::string, ASTNodeHandle> _literals{};
    uint32_t _lastOrder{};
};

template <typename NodeRef>
class ASTNodeRefInterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = NodeRef;
    using difference_type   = ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type;
    using const_pointer     = const value_type*;
    using const_reference   = const value_type;

    ASTNodeRefInterator() noexcept = default;

    explicit ASTNodeRefInterator(value_type nodeRef) noexcept : _current(nodeRef) {
    }

    [[nodiscard]] reference operator*() const noexcept {
        return _current;
    }

    [[nodiscard]] pointer operator->() noexcept {
        return &_current;
    }

    [[nodiscard]] const_pointer operator->() const noexcept {
        return &_current;
    }

    ASTNodeRefInterator& operator++() noexcept {
        _current = NodeRef(_current.ctx(), _current->sibling);
        return *this;
    }

    ASTNodeRefInterator operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    [[nodiscard]] bool operator==(const ASTNodeRefInterator& other) const noexcept {
        return _current == other._current;
    }

    [[nodiscard]] bool operator!=(const ASTNodeRefInterator& other) const noexcept {
        return !(*this == other);
    }

private:
    value_type _current{};
};

class ASTNodeRef {
public:
    using value_type      = ASTNodeRef;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

    using iterator       = ASTNodeRefInterator<ASTNodeRef>;
    using const_iterator = ASTNodeRefInterator<ASTNodeRef>;

    explicit ASTNodeRef() noexcept = default;

    explicit ASTNodeRef(Context& ctx) noexcept : _ctx(&ctx) {
    }

    ASTNodeRef(Context& ctx, ASTNodeHandle handle) noexcept :
        _ctx(&ctx),
        _handle(handle),
        _node(ctx.result()->getNode(_handle)) {
    }

    ASTNodeRef(const ASTNodeRef& node) noexcept : _ctx(node._ctx), _handle(node._handle), _node(node._node) {
    }

    ASTNodeRef(ASTNodeRef&& node) noexcept : _ctx(node._ctx), _handle(node._handle), _node(node._node) {
    }

    ASTNodeRef& operator=(const ASTNodeRef& node) noexcept {
        if (*this != node) {
            _ctx    = node._ctx;
            _handle = node._handle;
            _node   = node._node;
        }
        return *this;
    }

    ASTNodeRef& operator=(ASTNodeRef&& node) noexcept {
        if (*this != node) {
            _ctx    = node._ctx;
            _handle = node._handle;
            _node   = node._node;
        }
        return *this;
    }

    explicit operator bool() const noexcept {
        return _node != nullptr;
    }

    [[nodiscard]] bool operator==(const ASTNodeRef& node) const noexcept {
        return _handle == node._handle;
    }

    [[nodiscard]] bool operator!=(const ASTNodeRef& node) const noexcept {
        return !(*this == node);
    }

    [[nodiscard]] ASTNode& operator*() noexcept {
        return *_node;
    }

    [[nodiscard]] const ASTNode& operator*() const noexcept {
        return *_node;
    }

    [[nodiscard]] ASTNode* operator->() noexcept {
        return _node;
    }

    [[nodiscard]] const ASTNode* operator->() const noexcept {
        return _node;
    }

    [[nodiscard]] iterator begin() noexcept {
        return iterator(ASTNodeRef(*_ctx, _node ? _node->child : HandleNone));
    }

    [[nodiscard]] iterator end() noexcept {
        return iterator(ASTNodeRef(*_ctx, HandleNone));
    }

    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator(ASTNodeRef(*_ctx, _node ? _node->child : HandleNone));
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator(ASTNodeRef(*_ctx, HandleNone));
    }

    [[nodiscard]] const_iterator cbegin() const noexcept {
        return const_iterator(ASTNodeRef(*_ctx, _node ? _node->child : HandleNone));
    }

    [[nodiscard]] const_iterator cend() const noexcept {
        return const_iterator(ASTNodeRef(*_ctx, HandleNone));
    }

    [[nodiscard]] ASTNodeType type() const noexcept {
        return ASTNodeType(_node ? _node->type : 0);
    }

    template <ASTNodeType... Types>
    [[nodiscard]] bool is() const noexcept {
        return (isNodeType(_node, Types) || ...);
    }

    [[nodiscard]] Context& ctx() noexcept {
        return *_ctx;
    }

    [[nodiscard]] ASTNodeRef parent() const noexcept {
        return ASTNodeRef(*_ctx, _node ? _node->parent : HandleNone);
    }

    [[nodiscard]] ASTNodeHandle handle() const noexcept {
        return _handle;
    }

    [[nodiscard]] auto getChilds(bool originalNodes = false) const noexcept {
        return *this | std::views::filter([originalNodes](const auto& child) {
            if (child.changedByCompiler()) {
                if (child.replacedByCompiler() && originalNodes) {
                    return true;
                }
                if (child.addedByCompiler() && !originalNodes) {
                    return true;
                }
                return false;
            }
            if (child.is<IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF>()) {
                return false;
            }
            return true;
        });
    }

    [[nodiscard]] bool hasChilds() const noexcept {
        return _node ? _node->child != HandleNone : false;
    }

    void addChild(const ASTNodeRef& node) noexcept {
        if (!hasChilds()) {
            _node->child = node.handle();
        } else {
            auto it = begin();
            while (std::next(it) != end()) {
                ++it;
            }
            (*it)->sibling = node.handle();
        }
    }

    template <ASTNodeType Type>
    [[nodiscard]] ASTNodeRef findChild() const noexcept {
        auto view = *this | std::views::all;

        auto it = std::ranges::find_if(view, [](const auto& child) {
            return child.is<Type>();
        });

        return it != view.end() ? *it : ASTNodeRef(*_ctx);
    }

    [[nodiscard]] auto attrs() const noexcept {
        return *this | std::views::filter([](const auto& child) {
            return child.is<IDL_AST_NODE_TYPE_ATTR>();
        });
    }

    ASTNodeRef resolveRef(bool onlyType = false);

    template <typename Visitor, typename... Args>
    Visitor accept(Args&&... args) {
        Visitor visitor(std::forward<Args>(args)...);
        auto& node = *this;
        if (node) {
            switch (node->type) {
                case IDL_AST_NODE_TYPE_API:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_API>{});
                    break;
                case IDL_AST_NODE_TYPE_IMPORT:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_IMPORT>{});
                    break;
                case IDL_AST_NODE_TYPE_ENUM:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ENUM>{});
                    break;
                case IDL_AST_NODE_TYPE_CONST:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_CONST>{});
                    break;
                case IDL_AST_NODE_TYPE_DECL_REF:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_DECL_REF>{});
                    break;
                case IDL_AST_NODE_TYPE_LITERAL_STR:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_LITERAL_STR>{});
                    break;
                case IDL_AST_NODE_TYPE_LITERAL_INT:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_LITERAL_INT>{});
                    break;
                case IDL_AST_NODE_TYPE_LITERAL_BOOL:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_LITERAL_BOOL>{});
                    break;
                case IDL_AST_NODE_TYPE_LITERAL_FLOAT:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_LITERAL_FLOAT>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_FLAGS:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_FLAGS>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_HEX:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_HEX>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_VALUE:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_VALUE>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_TYPE:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_TYPE>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_CNAME:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_CNAME>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_TOKENIZER:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_ORDER:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_ORDER>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_SINGLE:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_SINGLE>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_VERSION:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_VERSION>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>{});
                    break;
                case IDL_AST_NODE_TYPE_VOID:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_VOID>{});
                    break;
                case IDL_AST_NODE_TYPE_DATA:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_DATA>{});
                    break;
                case IDL_AST_NODE_TYPE_CHAR:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_CHAR>{});
                    break;
                case IDL_AST_NODE_TYPE_STR:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_STR>{});
                    break;
                case IDL_AST_NODE_TYPE_BOOL:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_BOOL>{});
                    break;
                case IDL_AST_NODE_TYPE_INT_8:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_INT_8>{});
                    break;
                case IDL_AST_NODE_TYPE_UINT_8:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_UINT_8>{});
                    break;
                case IDL_AST_NODE_TYPE_INT_16:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_INT_16>{});
                    break;
                case IDL_AST_NODE_TYPE_UINT_16:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_UINT_16>{});
                    break;
                case IDL_AST_NODE_TYPE_INT_32:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_INT_32>{});
                    break;
                case IDL_AST_NODE_TYPE_UINT_32:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_UINT_32>{});
                    break;
                case IDL_AST_NODE_TYPE_INT_64:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_INT_64>{});
                    break;
                case IDL_AST_NODE_TYPE_UINT_64:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_UINT_64>{});
                    break;
                case IDL_AST_NODE_TYPE_FLOAT_32:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_FLOAT_32>{});
                    break;
                case IDL_AST_NODE_TYPE_FLOAT_64:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_FLOAT_64>{});
                    break;
            }
        }
        return visitor;
    }

    enum Filter {
        None         = 0,
        SkipDocs     = 1 << 0,
        SkipAttrs    = 1 << 1,
        SkipDecls    = 1 << 2,
        SkipImports  = 1 << 3,
        SkipLiterals = 1 << 4,
        SkipTrivials = 1 << 5,
        OriginalIdl  = 1 << 6
    };

    template <typename Visitor, typename... Args>
    void acceptRecursive(int filters, Args&&... args) {
        std::stack<ASTNodeRef> stack;
        stack.push(*this);

        std::vector<ASTNodeRef> buffer;
        buffer.reserve(50);

        while (!stack.empty()) {
            auto node = stack.top();
            stack.pop();

            if ((filters & SkipAttrs) == SkipAttrs && node.is<IDL_AST_NODE_TYPE_ATTR>()) {
                continue;
            }
            if ((filters & SkipDocs) == SkipDocs && node.is<IDL_AST_NODE_TYPE_ATTR_DOC>()) {
                continue;
            }
            if ((filters & SkipDecls) == SkipDecls && node.is<IDL_AST_NODE_TYPE_DECL>()) {
                continue;
            }
            if ((filters & SkipImports) == SkipImports && node.is<IDL_AST_NODE_TYPE_IMPORT>()) {
                continue;
            }
            if ((filters & SkipLiterals) == SkipLiterals && node.is<IDL_AST_NODE_TYPE_LITERAL>()) {
                continue;
            }
            if ((filters & SkipTrivials) == SkipTrivials && node.is<IDL_AST_NODE_TYPE_TRIVIAL_TYPE>()) {
                continue;
            }
            if (node.changedByCompiler()) {
                if ((filters & OriginalIdl) == OriginalIdl) {
                    if (node.addedByCompiler()) {
                        continue;
                    }
                } else {
                    if (node.replacedByCompiler()) {
                        continue;
                    }
                }
            }

            node.accept<Visitor>(std::forward<Args>(args)...);

            buffer.assign(node.begin(), node.end());
            for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
                stack.push(*it);
            }
        }
    }

    [[nodiscard]] bool evaulated() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_EVAULATED_BIT) == IDL_AST_NODE_STATE_EVAULATED_BIT : false;
    }

    void setEvaulated() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_EVAULATED_BIT;
        }
    }

    [[nodiscard]] bool buildError() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_BUILD_ERROR_BIT) == IDL_AST_NODE_STATE_BUILD_ERROR_BIT : true;
    }

    void setBuildError() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_BUILD_ERROR_BIT;
        }
    }

    [[nodiscard]] bool forwardDecl() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_FORWARD_DECL_BIT) == IDL_AST_NODE_STATE_FORWARD_DECL_BIT
                     : false;
    }

    void setForwardDecl() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_FORWARD_DECL_BIT;
        }
    }

    [[nodiscard]] bool addedByCompiler() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT) ==
                           IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT
                     : false;
    }

    void setAddedByCompiler() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT;
        }
    }

    [[nodiscard]] bool replacedByCompiler() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT) ==
                           IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT
                     : false;
    }

    void setReplacedByCompiler() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT;
        }
    }

    [[nodiscard]] bool changedByCompiler() const noexcept {
        return _node ? (_node->flags &
                        (IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT | IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT)) != 0
                     : false;
    }

    [[nodiscard]] std::string_view valueStr() const noexcept {
        return _ctx->result()->getStr(_node->valueStr);
    }

    [[nodiscard]] std::string_view name() const noexcept {
        return valueStr();
    }

    [[nodiscard]] std::string fullname() const {
        assert(is<IDL_AST_NODE_TYPE_DECL>());
        assert(name().length() > 0);
        std::string str{};
        if (auto prnt = parent()) {
            if (prnt.is<IDL_AST_NODE_TYPE_DECL>()) {
                str = prnt.fullname() + '.';
            }
        }
        if (is<IDL_AST_NODE_TYPE_IMPORT>()) {
            return str.substr(0, str.length() - 1);
        } else {
            auto nameView = name();
            return str + std::string(nameView.data(), nameView.length());
        }
    }

    [[nodiscard]] std::string fullnameLowercase() const {
        auto str = fullname();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        return str;
    }

    [[nodiscard]] ASTNodeRef declType() const noexcept {
        if (auto attrType = findChild<IDL_AST_NODE_TYPE_ATTR_TYPE>()) {
            if (auto declRef = ASTNodeRef(*_ctx, attrType->child)) {
                if (auto type = declRef.resolveRef(true)) {
                    return type;
                }
            }
        }
        return ASTNodeRef(*_ctx);
    }

    template <ASTNodeType Type>
    [[nodiscard]] static ASTNodeRef byType(Context& ctx) noexcept {
        static ASTNode node{};
        node.location = {};
        node.type     = Type;
        node.flags    = 0;
        node.parent   = HandleNone;
        node.sibling  = HandleNone;
        node.child    = HandleNone;
        ASTNodeRef ref(ctx);
        ref._node = &node;
        return ref;
    }

private:
    Context* _ctx{};
    ASTNodeHandle _handle{ HandleNone };
    ASTNode* _node{};
};

inline auto Context::getNodeRef(ASTNodeHandle handle) noexcept {
    return ASTNodeRef(*this, handle);
}

inline void Context::addChild(ASTNodeHandle parent, ASTNodeHandle child) noexcept {
    getNodeRef(parent).addChild(getNodeRef(child));
}

inline void Context::addSymbol(ASTNodeHandle decl) {
    std::queue<std::pair<ASTNodeRef, ASTNodeRef>> queue;
    queue.emplace(ASTNodeRef(*this), getNodeRef(decl));
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
        return;
    }
    const auto fullname = node.fullnameLowercase();
    if (_symbols.contains(fullname)) {
        log<IDL_STATUS_E3012>(node->location, node.fullname());
    }
    _symbols[fullname] = decl;
}

inline auto Context::findSymbol(ASTNodeRef& decl, const ASTLocation& loc, const std::string& name, bool onlyType) {
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
                return ASTNodeRef(*this);
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
        curr = curr.parent() ? curr.parent() : ASTNodeRef(*this);
        if (!curr.is<IDL_AST_NODE_TYPE_DECL>()) {
            curr = ASTNodeRef(*this);
        }
    }
    if (auto it = _symbols.find(nameLower); it != _symbols.end()) {
        auto symbol = getNodeRef(it->second);

        const auto expectedName = symbol.fullname();
        if (name != expectedName) {
            log<IDL_STATUS_E3036>(loc, name, expectedName);
            return ASTNodeRef(*this);
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
    return ASTNodeRef(*this);
}

inline void Context::initBuiltins(ASTNodeRef node) {
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
        auto view = _ctx->result()->getStr((*this)->valueDeclRef.symbol);
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
