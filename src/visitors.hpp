#ifndef IDL_VISITORS_HPP
#define IDL_VISITORS_HPP

#include "context.hpp"

namespace idl {

struct CName {
    explicit CName(Context& ctx) noexcept : ctx(ctx) {
    }

    /*void visit(ASTNode* node, Tag<ASTNodeType::Enum>) {
        str = cname(node);
        if (ctx.findChild<ASTNodeType::AttrFlags>(node)) {
            str += "_flags";
        }
        str += "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Const>) {
        str = cname(node, true);
        if (ctx.findChild<ASTNodeType::AttrFlags>(node->parent)) {
            str += "_BIT";
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Void>) {
        str = ctx.useStdTypes() ? "void" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Char>) {
        str = ctx.useStdTypes() ? "char" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Str>) {
        str = ctx.useStdTypes() ? "const char*" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Bool>) {
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

    void visit(ASTNode* node, Tag<ASTNodeType::Int8>) {
        str = ctx.useStdTypes() ? "int8_t" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Uint8>) {
        str = ctx.useStdTypes() ? "uint8_t" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Int16>) {
        str = ctx.useStdTypes() ? "int16_t" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Uint16>) {
        str = ctx.useStdTypes() ? "uint16_t" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Int32>) {
        str = ctx.useStdTypes() ? "int32_t" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Uint32>) {
        str = ctx.useStdTypes() ? "uint32_t" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Int64>) {
        str = ctx.useStdTypes() ? "int64_t" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Uint64>) {
        str = ctx.useStdTypes() ? "uint64_t" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Float32 {
        str = ctx.useStdTypes() ? "float" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Float64>) {
        str = ctx.useStdTypes() ? "double" : cname(node) + "_t";
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Data>) {
        str = ctx.useStdTypes() ? "void*" : cname(node) + "_t";
    }*/

    std::string cnameDecl(ASTNode* decl, bool upper) {
        assert(astNodeIs(decl, ASTNodeType::Decl));
        if (auto attr = ctx.findChild<ASTNodeType::AttrCName>(decl)) {
            auto str = ctx.getStr(attr->valueStr);
            return { str.data(), str.length() };
        }
        std::vector<int> nums{};
        if (auto attr = ctx.findChild<ASTNodeType::AttrTokenizer>(decl)) {
            auto curr = attr->child;
            while (curr != NodeHandleNone) {
                auto node = ctx.getNode(curr);
                nums.push_back(int(node->valueInt));
                curr = node->sibling;
            }
        }
        auto str = ctx.getStr(decl->valueStr);
        return convert({ str.data(), str.length() },
                       upper ? Case::ScreamingSnakeCase : Case::SnakeCase,
                       nums.empty() ? nullptr : &nums);
    }

    std::string cname(ASTNode* decl, bool upper = false) {
        assert(astNodeIs(decl, ASTNodeType::Decl));
        auto name = cnameDecl(decl, upper);
        if (auto parent = ctx.getNode(decl->parent); parent && astNodeIs(parent, ASTNodeType::Decl)) {
            return cname(parent, upper) + '_' + name;
        }
        return name;
    }

    Context& ctx;
    std::string str;
};

struct DeclToken {
    explicit DeclToken(Context& ctx) noexcept : ctx(ctx) {
    }

    void visit(ASTNode*, Tag<ASTNodeType::Api>) {
        str = "api";
    }

    void visit(ASTNode*, Tag<ASTNodeType::Enum>) {
        str = "enum";
    }

    void visit(ASTNode*, Tag<ASTNodeType::Const>) {
        str = "const";
    }

    void visit(ASTNode*, Tag<ASTNodeType::Import>) {
        str = "import";
    }

    template <ASTNodeType Type>
    void visit(ASTNode*, Tag<Type>) noexcept {
        assert(!"declaration name is missing");
    }

    Context& ctx;
    std::string str;
};

struct AttrName {
    explicit AttrName(Context& ctx) noexcept : ctx(ctx) {
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrTokenizer>) {
        str = "tokenizer";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrOrder>) {
        str = "order";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrSingle>) {
        str = "single";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrVersion>) {
        str = "version";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrDocAuthor>) {
        str = "author";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrDocCopyright>) {
        str = "copyright";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrDocLicense>) {
        str = "license";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrFlags>) {
        str = "flags";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrHex>) {
        str = "hex";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrDocBrief>) {
        str = "brief";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrDocDetail>) {
        str = "detail";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrValue>) {
        str = "value";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrType>) {
        str = "type";
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrCName>) {
        str = "cname";
    }

    template <ASTNodeType Type>
    void visit(ASTNode*, Tag<Type>) noexcept {
        assert(!"attribute name is missing");
    }

    Context& ctx;
    std::string str;
};

} // namespace idl

#endif
