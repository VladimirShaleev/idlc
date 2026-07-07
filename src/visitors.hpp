#ifndef IDL_VISITORS_HPP
#define IDL_VISITORS_HPP

#include "context.hpp"

namespace idl {

struct CName {
    /*void visit(ASTNodeRef& node, Tag<ASTNodeType::Enum>) {
        str = cname(node);
        if (ctx.findChild<ASTNodeType::AttrFlags>(node)) {
            str += "_flags";
        }
        str += "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Const>) {
        str = cname(node, true);
        if (ctx.findChild<ASTNodeType::AttrFlags>(node->parent)) {
            str += "_BIT";
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Void>) {
        str = ctx.useStdTypes() ? "void" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Char>) {
        str = ctx.useStdTypes() ? "char" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Str>) {
        str = ctx.useStdTypes() ? "const char*" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Bool>) {
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

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Int8>) {
        str = ctx.useStdTypes() ? "int8_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Uint8>) {
        str = ctx.useStdTypes() ? "uint8_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Int16>) {
        str = ctx.useStdTypes() ? "int16_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Uint16>) {
        str = ctx.useStdTypes() ? "uint16_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Int32>) {
        str = ctx.useStdTypes() ? "int32_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Uint32>) {
        str = ctx.useStdTypes() ? "uint32_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Int64>) {
        str = ctx.useStdTypes() ? "int64_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Uint64>) {
        str = ctx.useStdTypes() ? "uint64_t" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Float32 {
        str = ctx.useStdTypes() ? "float" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Float64>) {
        str = ctx.useStdTypes() ? "double" : cname(node) + "_t";
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Data>) {
        str = ctx.useStdTypes() ? "void*" : cname(node) + "_t";
    }*/

    std::string cnameDecl(ASTNodeRef& decl, bool upper) {
        assert(decl.is<ASTNodeType::Decl>());
        if (auto attr = decl.findChild<ASTNodeType::AttrCName>()) {
            auto str = attr.valueStr();
            return { str.data(), str.length() };
        }
        std::vector<int> nums{};
        if (auto attr = decl.findChild<ASTNodeType::AttrTokenizer>()) {
            auto view = attr | std::views::transform([](const auto& arg) {
                return int(arg->valueInt);
            });
            nums.assign(view.begin(), view.end());
        }
        auto str = decl.valueStr();
        return convert({ str.data(), str.length() },
                       upper ? Case::ScreamingSnakeCase : Case::SnakeCase,
                       nums.empty() ? nullptr : &nums);
    }

    std::string cname(ASTNodeRef& decl, bool upper = false) {
        auto name = cnameDecl(decl, upper);
        if (auto parent = decl.parent(); parent.is<ASTNodeType::Decl>()) {
            return cname(parent, upper) + '_' + name;
        }
        return name;
    }

    std::string str;
};

struct DeclToken {
    void visit(ASTNodeRef&, Tag<ASTNodeType::Api>) {
        str = "api";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::Enum>) {
        str = "enum";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::Const>) {
        str = "const";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::Import>) {
        str = "import";
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"declaration name is missing");
    }

    std::string str;
};

struct AttrName {
    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrTokenizer>) {
        str = "tokenizer";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrOrder>) {
        str = "order";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrSingle>) {
        str = "single";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrVersion>) {
        str = "version";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrDocAuthor>) {
        str = "author";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrDocCopyright>) {
        str = "copyright";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrDocLicense>) {
        str = "license";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrFlags>) {
        str = "flags";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrHex>) {
        str = "hex";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrDocBrief>) {
        str = "brief";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrDocDetail>) {
        str = "detail";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrValue>) {
        str = "value";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrType>) {
        str = "type";
    }

    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrCName>) {
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
