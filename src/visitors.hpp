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
        assert(!"Declaration name is missing");
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
        int i = 5;
    }

    Context& ctx;
    std::string str;
};

// struct ChildVisitor : Visitor {
//     enum Filter {
//         None         = 0,
//         SkipDocs     = 1 << 0,
//         SkipAttrs    = 1 << 1,
//         SkipDecls    = 1 << 2,
//         SkipImports  = 1 << 3,
//         SkipLiterals = 1 << 4,
//         SkipTrivials = 1 << 5,
//     };
//
//     ChildVisitor(Context& ctx, Visitor& visitor, int filters) noexcept :
//         Visitor(ctx),
//         visitor(visitor),
//         filters((Filter) filters) {
//     }
//
//     void visit(ASTLiteralStr* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTLiteralBool* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTLiteralInt* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTLiteralFloat* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTApi* node) override {
//         visitNode(node);
//         visitAttrs(node);
//         // visitDecls(node->decls.begin(), node->decls.end());
//     }
//
//     void visit(ASTEnum* node) override {
//         visitNode(node);
//         visitAttrs(node);
//         // visitDecls(node->consts.begin(), node->consts.end());
//     }
//
//     void visit(ASTConst* node) override {
//         visitNode(node);
//         visitAttrs(node);
//     }
//
//     void visit(ASTAttrTokenizer* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTAttrOrder* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTAttrSingle* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTAttrVersion* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTAttrAuthor* node) override {
//         visitNode(node);
//         visitDoc(node);
//     }
//
//     void visit(ASTAttrCopyright* node) override {
//         visitNode(node);
//         visitDoc(node);
//     }
//
//     void visit(ASTAttrLicense* node) override {
//         visitNode(node);
//         visitDoc(node);
//     }
//
//     void visit(ASTAttrFlags* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTAttrHex* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTAttrBrief* node) override {
//         visitNode(node);
//         visitDoc(node);
//     }
//
//     void visit(ASTAttrDetail* node) override {
//         visitNode(node);
//         visitDoc(node);
//     }
//
//     void visit(ASTAttrValue* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTAttrType* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTAttrCName* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTDeclRef* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTImport* node) override {
//         visitNode(node);
//         visitAttrs(node);
//         visitDecls(node->decls.begin(), node->decls.end());
//     }
//
//     void visit(ASTVoid* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTChar* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTStr* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTBool* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTInt8* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTUint8* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTInt16* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTUint16* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTInt32* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTUint32* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTInt64* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTUint64* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTFloat32* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTFloat64* node) override {
//         visitNode(node);
//     }
//
//     void visit(ASTData* node) override {
//         visitNode(node);
//     }
//
//     void visitNode(ASTNode* node) {
//         if (node) {
//             node->accept(visitor);
//         }
//     }
//
//     void visitAttrs(ASTDecl* decl) {
//         if (filters & SkipAttrs) {
//             return;
//         }
//         for (auto attr : decl->attrs()) {
//             if ((filters & SkipDocs) == SkipDocs && attr->as<ASTDocAttr>()) {
//                 continue;
//             }
//             attr->accept(*this);
//         }
//     }
//
//     template <typename It>
//     void visitDecls(It begin, It end) {
//         if (filters & SkipDecls) {
//             return;
//         }
//         for (auto it = begin; it != end; ++it) {
//             if ((filters & SkipImports) == SkipImports && (*it)->as<ASTImport>()) {
//                 continue;
//             }
//             if ((filters & SkipTrivials) == SkipTrivials && (*it)->as<ASTBuiltinType>()) {
//                 continue;
//             }
//             (*it)->accept(*this);
//         }
//     }
//
//     void visitDoc(ASTDocAttr* doc) {
//         if (filters & SkipDocs) {
//             return;
//         }
//         // for (auto message : doc->message) {
//         //     message->accept(*this);
//         // }
//     }
//
//     Visitor& visitor;
//     Filter filters;
// };

} // namespace idl

#endif
