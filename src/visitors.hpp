#ifndef IDL_VISITORS_HPP
#define IDL_VISITORS_HPP

#include "context.hpp"

namespace idl {

struct CName : Visitor {
    explicit CName(Context& ctx) noexcept : Visitor(ctx) {
    }

    void visit(ASTEnum* node) override {
        str = cname(node);
        if (node->findAttr<ASTAttrFlags>()) {
            str += "_flags";
        }
        str += "_t";
    }

    void visit(ASTConst* node) override {
        str = cname(node, true);
        if (node->parent->as<ASTDecl>()->findAttr<ASTAttrFlags>()) {
            str += "_BIT";
        }
    }

    void visit(ASTVoid* node) override {
        str = ctx.useStdTypes() ? "void" : cname(node) + "_t";
    }

    void visit(ASTChar* node) override {
        str = ctx.useStdTypes() ? "char" : cname(node) + "_t";
    }

    void visit(ASTStr* node) override {
        str = ctx.useStdTypes() ? "const char*" : cname(node) + "_t";
    }

    void visit(ASTBool* node) override {
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

    void visit(ASTInt8* node) override {
        str = ctx.useStdTypes() ? "int8_t" : cname(node) + "_t";
    }

    void visit(ASTUint8* node) override {
        str = ctx.useStdTypes() ? "uint8_t" : cname(node) + "_t";
    }

    void visit(ASTInt16* node) override {
        str = ctx.useStdTypes() ? "int16_t" : cname(node) + "_t";
    }

    void visit(ASTUint16* node) override {
        str = ctx.useStdTypes() ? "uint16_t" : cname(node) + "_t";
    }

    void visit(ASTInt32* node) override {
        str = ctx.useStdTypes() ? "int32_t" : cname(node) + "_t";
    }

    void visit(ASTUint32* node) override {
        str = ctx.useStdTypes() ? "uint32_t" : cname(node) + "_t";
    }

    void visit(ASTInt64* node) override {
        str = ctx.useStdTypes() ? "int64_t" : cname(node) + "_t";
    }

    void visit(ASTUint64* node) override {
        str = ctx.useStdTypes() ? "uint64_t" : cname(node) + "_t";
    }

    void visit(ASTFloat32* node) override {
        str = ctx.useStdTypes() ? "float" : cname(node) + "_t";
    }

    void visit(ASTFloat64* node) override {
        str = ctx.useStdTypes() ? "double" : cname(node) + "_t";
    }

    void visit(ASTData* node) override {
        str = ctx.useStdTypes() ? "void*" : cname(node) + "_t";
    }

    static std::string cnameDecl(ASTDecl* decl, bool upper) {
        if (auto attr = decl->findAttr<ASTAttrCName>()) {
            return attr->name;
        }
        std::vector<int>* nums = nullptr;
        if (auto attr = decl->findAttr<ASTAttrTokenizer>()) {
            nums = &attr->nums;
        }
        return convert(decl->name, upper ? Case::ScreamingSnakeCase : Case::SnakeCase, nums);
    }

    static std::string cname(ASTDecl* decl, bool upper = false) {
        auto name = cnameDecl(decl, upper);
        if (decl->parent) {
            if (auto parentDecl = decl->parent->as<ASTDecl>()) {
                return cname(parentDecl, upper) + '_' + name;
            }
        }
        return name;
    }

    std::string str;
    std::string native;
};

struct DeclToken : Visitor {
    explicit DeclToken(Context& ctx) noexcept : Visitor(ctx) {
    }

    void visit(ASTApi* node) override {
        str = "api";
    }

    void visit(ASTEnum* node) override {
        str = "enum";
    }

    void visit(ASTConst* node) override {
        str = "const";
    }

    void visit(ASTImport* node) override {
        str = "import";
    }

    void discarded(ASTNode*) override {
        assert(!"Declaration name is missing");
    }

    std::string str;
};

struct AttrName : Visitor {
    explicit AttrName(Context& ctx) noexcept : Visitor(ctx) {
    }

    void visit(ASTAttrTokenizer* node) override {
        str = "tokenizer";
    }

    void visit(ASTAttrOrder* node) override {
        str = "order";
    }

    void visit(ASTAttrSingle* node) override {
        str = "single";
    }

    void visit(ASTAttrVersion* node) override {
        str = "version";
    }

    void visit(ASTAttrAuthor* node) override {
        str = "author";
    }

    void visit(ASTAttrCopyright* node) override {
        str = "copyright";
    }

    void visit(ASTAttrLicense* node) override {
        str = "license";
    }

    void visit(ASTAttrFlags* node) override {
        str = "flags";
    }

    void visit(ASTAttrHex* node) override {
        str = "hex";
    }

    void visit(ASTAttrBrief* node) override {
        str = "brief";
    }

    void visit(ASTAttrDetail* node) override {
        str = "detail";
    }

    void visit(ASTAttrValue* node) override {
        str = "value";
    }

    void visit(ASTAttrType* node) override {
        str = "type";
    }

    void visit(ASTAttrCName* node) override {
        str = "cname";
    }

    void discarded(ASTNode*) override {
        assert(!"attribute name is missing");
    }

    std::string str;
};

} // namespace idl

#endif
