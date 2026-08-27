#ifndef AST_REF_HPP
#define AST_REF_HPP

#include "compilation_result.hpp"

namespace idl {

class Context;

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

    ASTNodeRef(Context& ctx) noexcept : _ctx(&ctx) {
    }

    ASTNodeRef(Context& ctx, ASTNodeHandle handle) noexcept : _ctx(&ctx), _handle(handle), _node(result()->getNode(_handle)) {
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

    [[nodiscard]] CompilationResultBase* result() noexcept;

    [[nodiscard]] const CompilationResultBase* result() const noexcept;

    [[nodiscard]] ASTNodeRef parent() const noexcept {
        return ASTNodeRef(*_ctx, _node ? _node->parent : HandleNone);
    }

    [[nodiscard]] ASTNodeHandle handle() const noexcept {
        return _handle;
    }

    [[nodiscard]] auto getChilds(bool originalNodes = false) const noexcept {
        return *this | std::views::filter([originalNodes](const auto& child) {
            return child.isSaveNode(originalNodes);
        });
    }

    template <ASTNodeType... Type>
    [[nodiscard]] auto getChilds(bool originalNodes = false) const noexcept {
        auto view = getChilds(originalNodes);
        return view | std::views::filter([originalNodes](const auto& child) {
            return child.template is<Type...>();
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

    template <ASTNodeType... Type>
    [[nodiscard]] ASTNodeRef findChild(bool originalNode = false) const noexcept {
        auto view = *this | std::views::all;

        auto it = std::ranges::find_if(view, [originalNode](const auto& child) {
            return child.isSaveNode(originalNode) && child.template is<Type...>();
        });

        return it != view.end() ? *it : ASTNodeRef(*_ctx);
    }

    [[nodiscard]] ASTNodeRef nextSibling(bool originalNode = false) const noexcept {
        if (!_node) {
            return ASTNodeRef(*_ctx);
        }
        auto next = ASTNodeRef(*_ctx, _node->sibling);
        while (next && !next.isSaveNode(originalNode)) {
            next = ASTNodeRef(*_ctx, next->sibling);
        }
        return next;
    }

    template <ASTNodeType... Type>
    [[nodiscard]] ASTNodeRef nextSibling(bool originalNode = false) const noexcept {
        auto next = nextSibling(originalNode);
        while (next && !next.template is<Type...>()) {
            next = next.nextSibling(originalNode);
        }
        return next;
    }

    [[nodiscard]] auto getAttrs(bool originalNodes = false) const noexcept {
        return getChilds(originalNodes) | std::views::filter([](const auto& child) {
            return child.template is<IDL_AST_NODE_TYPE_ATTR>();
        });
    }

    [[nodiscard]] ASTNodeRef resolveRef(bool onlyType = false);

    template <typename Visitor, typename... Args>
    Visitor accept(Args&&... args) {
        Visitor visitor(std::forward<Args>(args)...);
        ASTNodeRef node = *this;
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
                case IDL_AST_NODE_TYPE_STRUCT:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_STRUCT>{});
                    break;
                case IDL_AST_NODE_TYPE_FIELD:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_FIELD>{});
                    break;
                case IDL_AST_NODE_TYPE_FUNC:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_FUNC>{});
                    break;
                case IDL_AST_NODE_TYPE_ARG:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ARG>{});
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
                case IDL_AST_NODE_TYPE_ATTR_MAX_ENUM:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_MAX_ENUM>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_COUNT_ENUMS:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_COUNT_ENUMS>{});
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
                case IDL_AST_NODE_TYPE_ATTR_CCONV:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_CCONV>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_CFORMAT:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_CFORMAT>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_TOKENIZER:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_ARRAY:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_ARRAY>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_SINGLE:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_SINGLE>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_STD_TYPES:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_STD_TYPES>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_REF:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_REF>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_CONST:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_CONST>{});
                    break;
                case IDL_AST_NODE_TYPE_ATTR_OPTIONAL:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_OPTIONAL>{});
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
                case IDL_AST_NODE_TYPE_ATTR_DOC_RETURN:
                    visitor.visit(node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_RETURN>{});
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
        SkipBuiltins = 1 << 2,
        SkipDecls    = 1 << 3,
        SkipImports  = 1 << 4,
        SkipLiterals = 1 << 5,
        SkipTrivials = 1 << 6,
        OriginalIdl  = 1 << 7
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
            if ((filters & SkipBuiltins) == SkipBuiltins && node.builtin()) {
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

    [[nodiscard]] bool builtin() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_BUILTIN_BIT) == IDL_AST_NODE_STATE_BUILTIN_BIT : false;
    }

    void setBuiltin() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_BUILTIN_BIT;
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
        return _node ? (_node->flags & IDL_AST_NODE_STATE_FORWARD_DECL_BIT) == IDL_AST_NODE_STATE_FORWARD_DECL_BIT : false;
    }

    void setForwardDecl() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_FORWARD_DECL_BIT;
        }
    }

    [[nodiscard]] bool addedByCompiler() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT) == IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT : false;
    }

    void setAddedByCompiler() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT;
        }
    }

    [[nodiscard]] bool replacedByCompiler() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT) == IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT : false;
    }

    void setReplacedByCompiler() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT;
        }
    }

    [[nodiscard]] bool changedByCompiler() const noexcept {
        return _node ? (_node->flags & (IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT | IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT)) != 0 : false;
    }

    [[nodiscard]] bool multilineDoc() const noexcept {
        return _node ? (_node->flags & IDL_AST_NODE_STATE_MULTILINE_DOC_BIT) == IDL_AST_NODE_STATE_MULTILINE_DOC_BIT : false;
    }

    void setMultilineDoc() noexcept {
        if (_node) {
            _node->flags |= IDL_AST_NODE_STATE_MULTILINE_DOC_BIT;
        }
    }

    [[nodiscard]] std::string_view valueStr() const noexcept {
        return result()->getStr(_node->valueStr);
    }

    [[nodiscard]] std::string_view name() const noexcept {
        assert(is<IDL_AST_NODE_TYPE_DECL>());
        assert(_node->name.name.handle != 0);
        return result()->getStr(_node->name.name);
    }

    [[nodiscard]] std::string_view fullname() const {
        assert(is<IDL_AST_NODE_TYPE_DECL>());
        assert(_node->name.fullname.handle != 0);
        return result()->getStr(_node->name.fullname);
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

    template <idl_ast_node_type_t Type>
    ASTNodeRef addDeclType();

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
    bool isSaveNode(bool originalNode) const noexcept {
        if (changedByCompiler()) {
            if (replacedByCompiler() && originalNode) {
                return true;
            }
            if (addedByCompiler() && !originalNode) {
                return true;
            }
            return false;
        }
        return true;
    }

    Context* _ctx{};
    ASTNodeHandle _handle{ HandleNone };
    ASTNode* _node{};
};

} // namespace idl

#endif
