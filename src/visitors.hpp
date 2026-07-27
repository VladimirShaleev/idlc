#ifndef IDL_VISITORS_HPP
#define IDL_VISITORS_HPP

#include "context.hpp"

namespace idl {

struct CName {
    CName(bool includeImports = false, char separator = '_', std::optional<bool> isUpper = std::nullopt) noexcept :
        includeImports(includeImports),
        separator(separator),
        isUpper(isUpper) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        str = cname(node, false);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        str = cname(node);
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>()) {
            str += "_flags";
        }
        str += "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        str = cname(node, true);
        if (node.parent().findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>()) {
            str += "_BIT";
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        str = cname(node);
    }

    /*
    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_VOID>) {
        str = ctx.useStdTypes() ? "void" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CHAR>) {
        str = ctx.useStdTypes() ? "char" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_STR>) {
        str = ctx.useStdTypes() ? "const char*" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_BOOL>) {
        switch (ctx.boolType()) {
            case BoolType::Int32:
                str = ctx.useStdTypes() ? "uint32_t" : cname(node) + "_t";
                break;
            case BoolType::Int8:
                str = ctx.useStdTypes() ? "uint8_t" : cname(node) + "_t";
                break;
            case BoolType::StdBool:
                str = ctx.useStdTypes() ? "bool" : cname(node) + "_t";
                break;
            default:
                assert(!"unreachable code");
                break;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_INT_8>) {
        str = ctx.useStdTypes() ? "int8_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_UINT_8>) {
        str = ctx.useStdTypes() ? "uint8_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_INT_16>) {
        str = ctx.useStdTypes() ? "int16_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_UINT_16>) {
        str = ctx.useStdTypes() ? "uint16_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_INT_32>) {
        str = ctx.useStdTypes() ? "int32_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_UINT_32>) {
        str = ctx.useStdTypes() ? "uint32_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_INT_64>) {
        str = ctx.useStdTypes() ? "int64_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_UINT_64>) {
        str = ctx.useStdTypes() ? "uint64_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FLOAT_32 {
        str = ctx.useStdTypes() ? "float" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FLOAT_64>) {
        str = ctx.useStdTypes() ? "double" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_DATA>) {
        str = ctx.useStdTypes() ? "void*" : cname(node) + "_t";
    }*/

    std::string cnameDecl(ASTNodeRef& decl, bool upper) {
        assert(decl.is<IDL_AST_NODE_TYPE_DECL>());
        if (auto attr = decl.findChild<IDL_AST_NODE_TYPE_ATTR_CNAME>()) {
            auto str = decl.ctx().getNodeRef(attr->child).valueStr();
            return { str.data(), str.length() };
        }
        std::vector<int> nums{};
        if (auto attr = decl.findChild<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>()) {
            auto view = attr | std::views::transform([](const auto& arg) {
                return int(arg->valueInt);
            });
            nums.assign(view.begin(), view.end());
        }
        auto screaming = isUpper ? isUpper.value() : upper;
        auto str       = decl.name();
        return convert({ str.data(), str.length() },
                       screaming ? Case::ScreamingSnakeCase : Case::SnakeCase,
                       nums.empty() ? nullptr : &nums);
    }

    std::string cname(ASTNodeRef decl, bool upper = false) {
        FixedStack<std::string, 20> names;
        while (decl) {
            if (decl.is<IDL_AST_NODE_TYPE_DECL>()) {
                if (includeImports || !decl.is<IDL_AST_NODE_TYPE_IMPORT>()) {
                    names.push(cnameDecl(decl, upper));
                }
            }
            decl = decl.parent();
        }
        auto first = true;
        std::ostringstream ss;
        while (!names.empty()) {
            if (!first) {
                ss << separator;
            }
            ss << names.top();
            names.pop();
            first = false;
        }
        return ss.str();
    }

    std::string str;
    bool includeImports;
    char separator;
    std::optional<bool> isUpper;
};

struct CLiteral {
    explicit CLiteral(bool hex = false) noexcept : hex(hex) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_STR>) {
        auto view = node.valueStr();
        str.assign(view.begin(), view.end());
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_INT>) {
        auto type = node.ctx().emptyNodeRef();
        auto curr = node;
        while (curr) {
            if (type = curr.declType()) {
                break;
            }
            curr = curr.parent();
        }
        if (!type) {
            type = node.ctx().getTrivial<IDL_AST_NODE_TYPE_INT_64>();
        }

        auto isUnsigned = false;
        if (type.is<IDL_AST_NODE_TYPE_INTEGER_TYPE>()) {
            isUnsigned = type.is<IDL_AST_NODE_TYPE_UINT_8>() || type.is<IDL_AST_NODE_TYPE_UINT_16>() ||
                         type.is<IDL_AST_NODE_TYPE_UINT_32>() || type.is<IDL_AST_NODE_TYPE_UINT_64>();
        }

        if (isUnsigned) {
            str = hex ? fmt::format("{:#X}", uint64_t(node->valueInt)) : std::to_string(uint64_t(node->valueInt));
        } else {
            str = hex ? fmt::format("{:#X}", node->valueInt) : std::to_string(node->valueInt);
        }
        if (hex) {
            if (auto pos = str.find_first_of('X'); pos != std::string::npos) {
                str[pos] = 'x';
            }
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_DECL_REF>) {
        str = node.resolveRef().accept<CName>().str;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"literal is missing");
    }

    std::string str;
    bool hex;
};

struct PriorityDocAttr {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF> tag) {
        setPriority(tag);
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL> tag) {
        setPriority(tag);
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR> tag) {
        setPriority(tag);
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT> tag) {
        setPriority(tag);
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE> tag) {
        setPriority(tag);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    template <ASTNodeType Type>
    void setPriority(Tag<Type> tag) noexcept {
        prior = int(decltype(tag)::type);
    }

    int prior{ 1000 };
};

struct DeclToken {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_API>) {
        str = "api";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        str = "enum";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_CONST>) {
        str = "const";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_STRUCT>) {
        str = "struct";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_FIELD>) {
        str = "field";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        str = "func";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ARG>) {
        str = "arg";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        str = "import";
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"declaration name is missing");
    }

    std::string_view str;
};

struct AttrName {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>) {
        str = "tokenizer";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_ARRAY>) {
        str = "array";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_SINGLE>) {
        str = "single";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_VERSION>) {
        str = "version";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>) {
        str = "author";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>) {
        str = "copyright";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>) {
        str = "license";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_FLAGS>) {
        str = "flags";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_HEX>) {
        str = "hex";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>) {
        str = "brief";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>) {
        str = "detail";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_VALUE>) {
        str = "value";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_TYPE>) {
        str = "type";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_CNAME>) {
        str = "cname";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_REF>) {
        str = "ref";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_CONST>) {
        str = "const";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_OPTIONAL>) {
        str = "optional";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_MAX_ENUM>) {
        str = "maxenum";
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"attribute name is missing");
    }

    std::string_view str;
};

} // namespace idl

#endif
