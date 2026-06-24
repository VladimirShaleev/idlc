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
