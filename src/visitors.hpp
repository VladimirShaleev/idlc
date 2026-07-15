#ifndef IDL_VISITORS_HPP
#define IDL_VISITORS_HPP

#include "context.hpp"

namespace idl {

struct CName {
    /*void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        str = cname(node);
        if (ctx.findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>(node)) {
            str += "_flags";
        }
        str += "_t";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        str = cname(node, true);
        if (ctx.findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>(node->parent)) {
            str += "_BIT";
        }
    }

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
            auto str = attr.name();
            return { str.data(), str.length() };
        }
        std::vector<int> nums{};
        if (auto attr = decl.findChild<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>()) {
            auto view = attr | std::views::transform([](const auto& arg) {
                return int(arg->valueInt);
            });
            nums.assign(view.begin(), view.end());
        }
        auto str = decl.name();
        return convert({ str.data(), str.length() },
                       upper ? Case::ScreamingSnakeCase : Case::SnakeCase,
                       nums.empty() ? nullptr : &nums);
    }

    std::string cname(ASTNodeRef& decl, bool upper = false) {
        auto name = cnameDecl(decl, upper);
        if (auto parent = decl.parent(); parent.is<IDL_AST_NODE_TYPE_DECL>()) {
            return cname(parent, upper) + '_' + name;
        }
        return name;
    }

    std::string str;
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

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        str = "import";
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"declaration name is missing");
    }

    std::string str;
};

struct AttrName {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>) {
        str = "tokenizer";
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

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"attribute name is missing");
    }

    std::string str;
};

} // namespace idl

#endif
