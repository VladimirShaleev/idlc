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

class ASTNodeRef;

class Context final {
public:
    Context(Options* options, CompilationResult* result) noexcept : _options(options), _result(result) {
        _nodes.reserve(1024);
    }

    auto api() noexcept;

    bool useStdTypes() const noexcept {
        return _useStdTypes;
    }

    BoolType boolType() const noexcept {
        return _boolType;
    }

    ASTNode* getNode(ASTNodeHandle handle) noexcept {
        if (handle.handle < _nodes.size() && handle != NodeHandleNone) {
            return &_nodes[handle.handle];
        }
        return nullptr;
    }

    auto getNodeRef(ASTNodeHandle handle) noexcept;

    ASTNodeHandle allocNode(const idl::location& loc, ASTNodeType type) {
        auto index = _nodes.size();
        auto& node = _nodes.emplace_back();

        node.location.filename = _stringPool.insert({ loc.begin.filename->c_str(), loc.begin.filename->length() });
        node.location.line     = loc.begin.line;
        node.location.column   = loc.begin.column;
        node.type              = type;
        node.flags             = 0;
        node.parent            = NodeHandleNone;
        node.sibling           = NodeHandleNone;
        node.child             = NodeHandleNone;

        return { uint16_t(index) };
    }

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
        const auto message     = err<Status>(std::forward<Args>(args)...);
        const auto warnAsError = _options ? _options->getWarningsAsErrors() : false;
        _result->addMessage(Status, _stringPool[loc.filename], loc.line, loc.column, message, warnAsError);
    }

    bool hasErrors() const noexcept {
        return _api != NodeHandleNone ? _result->hasErrors() : true;
    }

    String intern(std::string_view str) {
        return _stringPool.insert(str);
    }

    std::string_view getStr(String str) const noexcept {
        return _stringPool[str];
    }

private:
    Options* _options;
    CompilationResult* _result;
    StringPool _stringPool;
    std::vector<idl_message_t> _messages{};
    std::optional<idl_api_version_t> _version{};
    bool _useStdTypes{};
    BoolType _boolType{};
    ASTNodeHandle _api{ NodeHandleNone };
    std::vector<ASTNode> _nodes{};
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

    ASTNodeRef(Context& ctx, ASTNodeHandle handle) noexcept : _ctx(&ctx), _handle(handle), _node(ctx.getNode(_handle)) {
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
        return iterator(ASTNodeRef(*_ctx, _node ? _node->child : NodeHandleNone));
    }

    [[nodiscard]] iterator end() noexcept {
        return iterator(ASTNodeRef(*_ctx, NodeHandleNone));
    }

    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator(ASTNodeRef(*_ctx, _node ? _node->child : NodeHandleNone));
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator(ASTNodeRef(*_ctx, NodeHandleNone));
    }

    [[nodiscard]] const_iterator cbegin() const noexcept {
        return const_iterator(ASTNodeRef(*_ctx, _node ? _node->child : NodeHandleNone));
    }

    [[nodiscard]] const_iterator cend() const noexcept {
        return const_iterator(ASTNodeRef(*_ctx, NodeHandleNone));
    }

    template <ASTNodeType... Types>
    [[nodiscard]] bool is() const noexcept {
        return (astNodeIs(_node, Types) || ...);
    }

    [[nodiscard]] Context& ctx() noexcept {
        return *_ctx;
    }

    [[nodiscard]] ASTNodeRef parent() const noexcept {
        return ASTNodeRef(*_ctx, _node ? _node->parent : NodeHandleNone);
    }

    [[nodiscard]] ASTNodeHandle handle() const noexcept {
        return _handle;
    }

    [[nodiscard]] bool hasChilds() const noexcept {
        return _node ? _node->child != NodeHandleNone : false;
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
            return child.is<ASTNodeType::Attr>();
        });
    }

    ASTNodeRef resolveRef(ASTNodeRef decl, bool onlyType = false);

    template <typename Visitor, typename... Args>
    Visitor accept(Args&&... args) {
        Visitor visitor(std::forward<Args>(args)...);
        auto& node = *this;
        if (node) {
            switch (node->type) {
                case ASTNodeType::Api:
                    visitor.visit(node, Tag<ASTNodeType::Api>{});
                    break;
                case ASTNodeType::Import:
                    visitor.visit(node, Tag<ASTNodeType::Import>{});
                    break;
                case ASTNodeType::Enum:
                    visitor.visit(node, Tag<ASTNodeType::Enum>{});
                    break;
                case ASTNodeType::Const:
                    visitor.visit(node, Tag<ASTNodeType::Const>{});
                    break;
                case ASTNodeType::DeclRef:
                    visitor.visit(node, Tag<ASTNodeType::DeclRef>{});
                    break;
                case ASTNodeType::LiteralStr:
                    visitor.visit(node, Tag<ASTNodeType::LiteralStr>{});
                    break;
                case ASTNodeType::LiteralInt:
                    visitor.visit(node, Tag<ASTNodeType::LiteralInt>{});
                    break;
                case ASTNodeType::LiteralBool:
                    visitor.visit(node, Tag<ASTNodeType::LiteralBool>{});
                    break;
                case ASTNodeType::LiteralFloat:
                    visitor.visit(node, Tag<ASTNodeType::LiteralFloat>{});
                    break;
                case ASTNodeType::AttrFlags:
                    visitor.visit(node, Tag<ASTNodeType::AttrFlags>{});
                    break;
                case ASTNodeType::AttrHex:
                    visitor.visit(node, Tag<ASTNodeType::AttrHex>{});
                    break;
                case ASTNodeType::AttrValue:
                    visitor.visit(node, Tag<ASTNodeType::AttrValue>{});
                    break;
                case ASTNodeType::AttrType:
                    visitor.visit(node, Tag<ASTNodeType::AttrType>{});
                    break;
                case ASTNodeType::AttrCName:
                    visitor.visit(node, Tag<ASTNodeType::AttrCName>{});
                    break;
                case ASTNodeType::AttrTokenizer:
                    visitor.visit(node, Tag<ASTNodeType::AttrTokenizer>{});
                    break;
                case ASTNodeType::AttrOrder:
                    visitor.visit(node, Tag<ASTNodeType::AttrOrder>{});
                    break;
                case ASTNodeType::AttrSingle:
                    visitor.visit(node, Tag<ASTNodeType::AttrSingle>{});
                    break;
                case ASTNodeType::AttrVersion:
                    visitor.visit(node, Tag<ASTNodeType::AttrVersion>{});
                    break;
                case ASTNodeType::AttrDocBrief:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocBrief>{});
                    break;
                case ASTNodeType::AttrDocDetail:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocDetail>{});
                    break;
                case ASTNodeType::AttrDocAuthor:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocAuthor>{});
                    break;
                case ASTNodeType::AttrDocCopyright:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocCopyright>{});
                    break;
                case ASTNodeType::AttrDocLicense:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocLicense>{});
                    break;
                case ASTNodeType::Void:
                    visitor.visit(node, Tag<ASTNodeType::Void>{});
                    break;
                case ASTNodeType::Data:
                    visitor.visit(node, Tag<ASTNodeType::Data>{});
                    break;
                case ASTNodeType::Char:
                    visitor.visit(node, Tag<ASTNodeType::Char>{});
                    break;
                case ASTNodeType::Str:
                    visitor.visit(node, Tag<ASTNodeType::Str>{});
                    break;
                case ASTNodeType::Bool:
                    visitor.visit(node, Tag<ASTNodeType::Bool>{});
                    break;
                case ASTNodeType::Int8:
                    visitor.visit(node, Tag<ASTNodeType::Int8>{});
                    break;
                case ASTNodeType::Uint8:
                    visitor.visit(node, Tag<ASTNodeType::Uint8>{});
                    break;
                case ASTNodeType::Int16:
                    visitor.visit(node, Tag<ASTNodeType::Int16>{});
                    break;
                case ASTNodeType::Uint16:
                    visitor.visit(node, Tag<ASTNodeType::Uint16>{});
                    break;
                case ASTNodeType::Int32:
                    visitor.visit(node, Tag<ASTNodeType::Int32>{});
                    break;
                case ASTNodeType::Uint32:
                    visitor.visit(node, Tag<ASTNodeType::Uint32>{});
                    break;
                case ASTNodeType::Int64:
                    visitor.visit(node, Tag<ASTNodeType::Int64>{});
                    break;
                case ASTNodeType::Uint64:
                    visitor.visit(node, Tag<ASTNodeType::Uint64>{});
                    break;
                case ASTNodeType::Float32:
                    visitor.visit(node, Tag<ASTNodeType::Float32>{});
                    break;
                case ASTNodeType::Float64:
                    visitor.visit(node, Tag<ASTNodeType::Float64>{});
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

            if ((filters & SkipAttrs) == SkipAttrs && node.is<ASTNodeType::Attr>()) {
                continue;
            }
            if ((filters & SkipDocs) == SkipDocs && node.is<ASTNodeType::AttrDoc>()) {
                continue;
            }
            if ((filters & SkipDecls) == SkipDecls && node.is<ASTNodeType::Decl>()) {
                continue;
            }
            if ((filters & SkipImports) == SkipImports && node.is<ASTNodeType::Import>()) {
                continue;
            }
            if ((filters & SkipLiterals) == SkipLiterals && node.is<ASTNodeType::Literal>()) {
                continue;
            }
            if ((filters & SkipTrivials) == SkipTrivials && node.is<ASTNodeType::TrivialType>()) {
                continue;
            }

            node.accept<Visitor>(std::forward<Args>(args)...);

            buffer.assign(node.begin(), node.end());
            for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
                stack.push(*it);
            }
        }
    }

    [[nodiscard]] bool evaulated() const noexcept {
        return _node ? (_node->flags & ASTNODE_EVAULATED) == ASTNODE_EVAULATED : false;
    }

    [[nodiscard]] bool buildError() const noexcept {
        return _node ? (_node->flags & ASTNODE_BUILD_ERROR) == ASTNODE_BUILD_ERROR : true;
    }

    [[nodiscard]] bool forwardDecl() const noexcept {
        return _node ? (_node->flags & ASTNODE_FORWARD_DECL) == ASTNODE_FORWARD_DECL : false;
    }

    [[nodiscard]] std::string_view valueStr() const noexcept {
        return _ctx->getStr(_node->valueStr);
    }

    [[nodiscard]] ASTNodeRef valueNode() const noexcept {
        return ASTNodeRef(*_ctx, _node ? _node->valueHandle : NodeHandleNone);
    }

    [[nodiscard]] std::string fullname() const {
        assert(is<ASTNodeType::Decl>());
        assert(valueStr().length() > 0);
        std::string str{};
        if (auto prnt = parent()) {
            if (prnt.is<ASTNodeType::Decl>()) {
                str = prnt.fullname() + '.';
            }
        }
        if (is<ASTNodeType::Import>()) {
            return str.substr(0, str.length() - 1);
        } else {
            auto name = valueStr();
            return str + std::string(name.data(), name.length());
        }
    }

    [[nodiscard]] std::string fullnameLowercase() const {
        auto str = fullname();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        return str;
    }

    template <ASTNodeType Type>
    [[nodiscard]] static ASTNodeRef byType(Context& ctx) noexcept {
        static ASTNode node{};
        node.location = {};
        node.type     = Type;
        node.flags    = 0;
        node.parent   = NodeHandleNone;
        node.sibling  = NodeHandleNone;
        node.child    = NodeHandleNone;
        ASTNodeRef ref(ctx);
        ref._node = &node;
        return ref;
    }

private:
    Context* _ctx{};
    ASTNodeHandle _handle{ NodeHandleNone };
    ASTNode* _node{};
};

inline auto Context::getNodeRef(ASTNodeHandle handle) noexcept {
    return ASTNodeRef(*this, handle);
}

inline auto Context::api() noexcept {
    return getNodeRef(_api);
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
    if (node.is<ASTNodeType::Import>()) {
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
    auto curr   = decl;
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
            if (onlyType) {
                if (symbol.is<ASTNodeType::Type>()) {
                    return symbol;
                }
            } else {
                return symbol;
            }
        }
        curr = curr.parent() ? curr.parent() : ASTNodeRef(*this);
        if (!curr.is<ASTNodeType::Decl>()) {
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
        if (onlyType) {
            if (symbol.is<ASTNodeType::Type>()) {
                return symbol;
            }
        } else {
            return symbol;
        }
    }
    if (onlyType) {
        log<IDL_STATUS_E3043>(loc, name);
    } else {
        log<IDL_STATUS_E3037>(loc, name);
    }
    return ASTNodeRef(*this);
}

inline void Context::initBuiltins(ASTNodeRef node) {
    _api = node.handle();

    static const std::string filename = "<builtin>";

    const auto loc     = idl::location(idl::position(&filename, 1, 1));
    ASTNodeHandle last = NodeHandleNone;

    auto addBuiltin =
        [this, &loc, &last]<ASTNodeType Type>(std::string_view name, const std::string& detail, Tag<Type>) {
        auto node               = allocNode(loc, Tag<Type>::type);
        getNode(node)->valueStr = intern(name);
        getNode(node)->parent   = _api;
        addSymbol(node);

        if (last == NodeHandleNone) {
            if (getNode(_api)->child == NodeHandleNone) {
                getNode(_api)->child = node;
            } else {
                auto lastChild = NodeHandleNone;
                auto currChild = getNode(_api)->child;
                while (currChild != NodeHandleNone) {
                    auto nodeChild = getNode(currChild);
                    lastChild      = currChild;
                    currChild      = nodeChild->sibling;
                }
                getNode(lastChild)->sibling = node;
            }
        } else {
            getNode(last)->sibling = node;
        }
        last = node;
    };

    addBuiltin("Void", "void type.", Tag<ASTNodeType::Void>{});
    addBuiltin("Char", "symbol type.", Tag<ASTNodeType::Char>{});
    addBuiltin("Bool", "boolean type.", Tag<ASTNodeType::Bool>{});
    addBuiltin("Int8", "8 bit signed integer.", Tag<ASTNodeType::Int8>{});
    addBuiltin("Uint8", "8 bit unsigned integer.", Tag<ASTNodeType::Uint8>{});
    addBuiltin("Int16", "16 bit signed integer.", Tag<ASTNodeType::Int16>{});
    addBuiltin("Uint16", "16 bit unsigned integer.", Tag<ASTNodeType::Uint16>{});
    addBuiltin("Int32", "32 bit signed integer.", Tag<ASTNodeType::Int32>{});
    addBuiltin("Uint32", "32 bit unsigned integer.", Tag<ASTNodeType::Uint32>{});
    addBuiltin("Int64", "64 bit signed integer.", Tag<ASTNodeType::Int64>{});
    addBuiltin("Uint64", "64 bit unsigned integer.", Tag<ASTNodeType::Uint64>{});
    addBuiltin("Float32", "32 bit float point.", Tag<ASTNodeType::Float32>{});
    addBuiltin("Float64", "64 bit float point.", Tag<ASTNodeType::Float64>{});
    addBuiltin("Str", "utf8 string.", Tag<ASTNodeType::Str>{});
    addBuiltin("Data", "pointer to data.", Tag<ASTNodeType::Data>{});
}

inline ASTNodeRef ASTNodeRef::resolveRef(ASTNodeRef decl, bool onlyType) {
    if (!evaulated()) {
        (*this)->flags |= ASTNODE_EVAULATED;
        auto view = valueStr();
        std::string name(view.data(), view.length());
        if (auto symbol = _ctx->findSymbol(decl, (*this)->location, name, onlyType)) {
            (*this)->valueHandle = symbol.handle();
            return symbol;
        } else {
            (*this)->valueHandle = NodeHandleNone;
        }
    }
    return ASTNodeRef(*_ctx, (*this)->valueHandle);
}

} // namespace idl

#endif
