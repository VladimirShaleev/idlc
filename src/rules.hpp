#ifndef IDL_RULES_HPP
#define IDL_RULES_HPP

#include "context.hpp"

namespace idl {

struct CName : Visitor {
    void discarded(ASTNode*) override {
        assert(!"C name is missing");
    }

    static std::string cnameDecl(ASTDecl* decl, bool upper) {
        // if (auto attr = decl->findAttr<ASTAttrCName>()) {
        //     return attr->name;
        // }
        std::vector<int>* nums = nullptr;
        // if (auto attr = decl->findAttr<ASTAttrTokenizer>()) {
        //     nums = &attr->nums;
        // }
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

    void discarded(ASTNode*) override {
        assert(!"Declaration name is missing");
    }

    std::string str;
};

struct AttrName : Visitor {
    explicit AttrName(Context& ctx) noexcept : Visitor(ctx) {
    }

    // void visit(ASTAttrPlatform*) override {
    //     str = "platform";
    // }

    // void visit(ASTAttrFlags*) override {
    //     str = "flags";
    // }

    // void visit(ASTAttrHex*) override {
    //     str = "hex";
    // }

    // void visit(ASTAttrValue*) override {
    //     str = "value";
    // }

    // void visit(ASTAttrType*) override {
    //     str = "type";
    // }

    // void visit(ASTAttrStatic* node) override {
    //     str = "static";
    // }

    // void visit(ASTAttrCtor* node) override {
    //     str = "ctor";
    // }

    // void visit(ASTAttrThis* node) override {
    //     str = "this";
    // }

    // void visit(ASTAttrGet* node) override {
    //     str = "get";
    // }

    // void visit(ASTAttrSet* node) override {
    //     str = "set";
    // }

    // void visit(ASTAttrHandle* node) override {
    //     str = "handle";
    // }

    // void visit(ASTAttrCName* node) override {
    //     str = "cname";
    // }

    // void visit(ASTAttrArray* node) override {
    //     str = "array";
    // }

    // void visit(ASTAttrDataSize* node) override {
    //     str = "datasize";
    // }

    // void visit(ASTAttrConst* node) override {
    //     str = "const";
    // }

    // void visit(ASTAttrRef* node) override {
    //     str = "ref";
    // }

    // void visit(ASTAttrRefInc* node) override {
    //     str = "refinc";
    // }

    // void visit(ASTAttrUserData* node) override {
    //     str = "userdata";
    // }

    // void visit(ASTAttrErrorCode* node) override {
    //     str = "errorcode";
    // }

    // void visit(ASTAttrNoError* node) override {
    //     str = "noerror";
    // }

    // void visit(ASTAttrResult* node) override {
    //     str = "result";
    // }

    // void visit(ASTAttrDestroy* node) override {
    //     str = "destroy";
    // }

    // void visit(ASTAttrIn* node) override {
    //     str = "in";
    // }

    // void visit(ASTAttrOut* node) override {
    //     str = "out";
    // }

    // void visit(ASTAttrOptional* node) override {
    //     str = "optional";
    // }

    // void visit(ASTAttrTokenizer* node) override {
    //     str = "tokenizer";
    // }

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

    void discarded(ASTNode*) override {
        assert(!"attribute name is missing");
    }

    std::string str;
};

struct AttrValidatorRules : Visitor {
    explicit AttrValidatorRules(Context& ctx) noexcept : Visitor(ctx) {
    }

    // void visit(ASTEnum*) override {
    //     allowed = { add<ASTAttrFlags>(), add<ASTAttrHex>(),       add<ASTAttrPlatform>(),
    //                 add<ASTAttrCName>(), add<ASTAttrTokenizer>(), add<ASTAttrErrorCode>() };
    // }

    // void visit(ASTEnumConst*) override {
    //     allowed = {
    //         add<ASTAttrType>(), add<ASTAttrValue>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>(),
    //         add<ASTAttrNoError>()
    //     };
    // }

    // void visit(ASTStruct* node) override {
    //     allowed = { add<ASTAttrPlatform>(), add<ASTAttrHandle>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
    // }

    // void visit(ASTField* node) override {
    //     allowed = { add<ASTAttrType>(),  add<ASTAttrValue>(),    add<ASTAttrCName>(), add<ASTAttrTokenizer>(),
    //                 add<ASTAttrArray>(), add<ASTAttrDataSize>(), add<ASTAttrConst>(), add<ASTAttrRef>() };
    // }

    // void visit(ASTInterface* node) override {
    //     allowed = { add<ASTAttrPlatform>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
    // }

    // void visit(ASTHandle* node) override {
    //     allowed = { add<ASTAttrPlatform>(), add<ASTAttrType>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
    // }

    // void visit(ASTMethod* node) override {
    //     allowed = { add<ASTAttrType>(),    add<ASTAttrPlatform>(),  add<ASTAttrStatic>(),  add<ASTAttrCtor>(),
    //                 add<ASTAttrCName>(),   add<ASTAttrTokenizer>(), add<ASTAttrConst>(),   add<ASTAttrRefInc>(),
    //                 add<ASTAttrDestroy>(), add<ASTAttrRef>(),       add<ASTAttrOptional>() };
    // }

    // void visit(ASTProperty* node) override {
    //     allowed = { add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrStatic>(),   add<ASTAttrGet>(),
    //                 add<ASTAttrSet>(),  add<ASTAttrCName>(),    add<ASTAttrTokenizer>() };
    // }

    // void visit(ASTEvent* node) override {
    //     allowed = { add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrStatic>(),   add<ASTAttrGet>(),
    //                 add<ASTAttrSet>(),  add<ASTAttrCName>(),    add<ASTAttrTokenizer>() };
    // }

    // void visit(ASTArg* node) override {
    //     allowed = { add<ASTAttrType>(),      add<ASTAttrValue>(),   add<ASTAttrThis>(), add<ASTAttrCName>(),
    //                 add<ASTAttrTokenizer>(), add<ASTAttrConst>(),   add<ASTAttrRef>(),  add<ASTAttrUserData>(),
    //                 add<ASTAttrResult>(),    add<ASTAttrIn>(),      add<ASTAttrOut>(),  add<ASTAttrArray>(),
    //                 add<ASTAttrDataSize>(),  add<ASTAttrOptional>() };
    // }

    void visit(ASTApi* node) override {
        static std::map<std::type_index, std::string> allowed = { add<ASTAttrVersion>(), add<ASTAttrBrief>() };
        validate(node, allowed, node->attrs);
    }

    void visit(ASTEnum* node) override {
        static std::map<std::type_index, std::string> allowed = { add<ASTAttrFlags>(),
                                                                  add<ASTAttrHex>(),
                                                                  add<ASTAttrBrief>() };
        validate(node, allowed, node->attrs);
    }

    // void visit(ASTFunc* node) override {
    //     allowed = { add<ASTAttrType>(),      add<ASTAttrPlatform>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>(),
    //                 add<ASTAttrErrorCode>(), add<ASTAttrRef>(),      add<ASTAttrConst>() };
    // }

    // void visit(ASTCallback* node) override {
    //     allowed = { add<ASTAttrType>(), add<ASTAttrPlatform>(), add<ASTAttrCName>(),   add<ASTAttrTokenizer>(),
    //                 add<ASTAttrRef>(),  add<ASTAttrConst>(),    add<ASTAttrOptional>() };
    // }

    void discarded(ASTNode* node) {
        if (auto decl = node->as<ASTDecl>()) {
            DeclToken token(ctx);
            node->accept(token);
            ctx.log<IDL_STATUS_E3006>(decl->location, token.str, decl->fullname());
        } else {
            assert(!"attempt to validate attributes for a non-declaration node");
        }
    }

    void validate(ASTDecl* decl,
                  const std::map<std::type_index, std::string>& allowed,
                  const std::vector<ASTAttr*>& attrs) {
        auto names = fmt::format("{}", fmt::join(std::views::values(allowed), ", "));
        std::set<std::type_index> uniqueAttrs;
        for (auto& attr : attrs) {
            if (!attr) {
                continue;
            }
            if (!allowed.contains(typeid(*attr))) {
                AttrName name(ctx);
                attr->accept(name);
                DeclToken token(ctx);
                decl->accept(token);
                ctx.log<IDL_STATUS_E3005>(attr->location, name.str, token.str, decl->fullname(), names);
            }
            if (!uniqueAttrs.insert(typeid(*attr)).second) {
                AttrName name(ctx);
                attr->accept(name);
                DeclToken token(ctx);
                decl->accept(token);
                ctx.log<IDL_STATUS_E3007>(attr->location, name.str, token.str, decl->fullname());
            }
        }
    }

    template <typename Attr>
    std::pair<std::type_index, std::string> add() {
        return { typeid(Attr), getName<Attr>() };
    }

    template <typename Attr>
    std::string getName() {
        AttrName name(ctx);
        Attr attr{};
        attr.accept(name);
        return name.str;
    }
};

struct ApiChildAdder : Visitor {
    explicit ApiChildAdder(Context& ctx, ASTApi* api) noexcept : Visitor(ctx), api(api) {
    }

    void visit(ASTEnum* node) override {
        api->enums.push_back(node);
    }

    void discarded(ASTNode* node) override {
        assert(!"attempt to add an invalid node to the ASTApi");
    }

    ASTApi* api;
};

struct ChildAdder : Visitor {
    explicit ChildAdder(Context& ctx, ASTNode* child) noexcept : Visitor(ctx), child(child) {
    }

    void visit(ASTApi* node) override {
        ApiChildAdder adder(ctx, node);
        child->accept(adder);
    }

    void discarded(ASTNode* node) override {
        assert(!"attempt to add an invalid node to the ASTDecl");
    }

    ASTNode* child;
};

struct AttrArgRules : Visitor {
    AttrArgRules(Context& ctx, const std::vector<ASTNode*>& args) noexcept : Visitor(ctx), args(args) {
    }

    void visit(ASTAttrVersion* node) override {
        if (args.size() == 3 && args[0] && args[1] && args[2] && args[0]->as<ASTLiteralInt>() &&
            args[1]->as<ASTLiteralInt>() && args[2]->as<ASTLiteralInt>()) {
            const auto major = args[0]->as<ASTLiteralInt>()->value;
            const auto minor = args[1]->as<ASTLiteralInt>()->value;
            const auto micro = args[2]->as<ASTLiteralInt>()->value;
            auto fail        = false;
            if (major > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, major);
                fail = true;
            }
            if (minor > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, minor);
                fail = true;
            }
            if (micro > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, micro);
                fail = true;
            }
            if (!fail) {
                node->version = ASTAttrVersion::Semver{ (uint8_t) major, (uint8_t) minor, (uint8_t) micro };
            }
        } else if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralStr>()) {
            node->version = args[0]->as<ASTLiteralStr>()->value;
        } else {
            ctx.log<IDL_STATUS_E3003>(node->location);
        }
    }

    void visit(ASTAttrBrief* node) override {
        if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralStr>()) {
            node->message = args[0]->as<ASTLiteralStr>()->value;
        } else {
            ctx.log<IDL_STATUS_E3014>(node->location);
        }
    }

    void discarded(ASTNode* node) override {
        if (!args.empty()) {
            AttrName name(ctx);
            node->accept(name);
            ctx.log<IDL_STATUS_E3008>(node->location, name.str);
        }
    }

    const std::vector<ASTNode*>& args;
};

struct HierarchyRules : Visitor {
    HierarchyRules(Context& ctx) noexcept : Visitor(ctx) {
    }

    void visit(ASTApi* node) override {
        if (ctx._api != nullptr) {
            ctx.log<IDL_STATUS_E3010>(node->location, node->fullname());
        } else {
            ctx._api = node;
        }
    }

    void visit(ASTEnum* node) override {
        if (checkApi()) {
            node->parent = ctx.api();
            ctx.api()->enums.push_back(node);
        }
    }

    bool checkApi() {
        if (!ctx.api()) {
            ctx.log<IDL_STATUS_E3011>(idl::location());
            return false;
        }
        return true;
    }

    void addAttributes(ASTDecl* decl) noexcept {
        for (auto attr : decl->attrs) {
            attr->parent = decl;
        }
    }
};

} // namespace idl

#endif
