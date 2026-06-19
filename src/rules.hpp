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

    void visit(ASTAttrDetail* node) override {
        str = "detail";
    }

    void visit(ASTAttrValue* node) override {
        str = "value";
    }

    void visit(ASTAttrType* node) override {
        str = "type";
    }

    void discarded(ASTNode*) override {
        assert(!"attribute name is missing");
    }

    std::string str;
};

struct AttrValueOrTypeRules : Visitor {
    explicit AttrValueOrTypeRules(Context& ctx) noexcept : Visitor(ctx) {
    }

    void visit(ASTConst*) override {
        isValue = true;
    }

    void discarded(ASTNode*) override {
    }

    bool isValue{};
};

struct AttrValidatorRules : Visitor {
    struct AttrInfo {
        std::string name;
        bool recommended;
    };

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
        static std::map allowed = { add<ASTAttrVersion>(true), add<ASTAttrBrief>(true), add<ASTAttrDetail>(true) };
        validate(node, allowed, node->attrs);
    }

    void visit(ASTImport* node) override {
        static std::map allowed = { add<ASTAttrBrief>(true), add<ASTAttrDetail>(true) };
        validate(node, allowed, node->attrs);
    }

    void visit(ASTEnum* node) override {
        static std::map allowed = {
            add<ASTAttrFlags>(), add<ASTAttrHex>(), add<ASTAttrBrief>(true), add<ASTAttrDetail>(true)
        };
        validate(node, allowed, node->attrs);
    }

    void visit(ASTConst* node) override {
        static std::map allowed = { add<ASTAttrDetail>(true), add<ASTAttrValue>() };
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

    void discarded(ASTNode* node) override {
        if (auto decl = node->as<ASTDecl>()) {
            DeclToken token(ctx);
            node->accept(token);
            ctx.log<IDL_STATUS_E3006>(decl->location, token.str, decl->fullname());
        } else {
            assert(!"attempt to validate attributes for a non-declaration node");
        }
    }

    void validate(ASTDecl* decl,
                  const std::map<std::type_index, AttrInfo>& allowed,
                  const std::vector<ASTAttr*>& attrs) {
        auto fieldNames =
            allowed | std::views::values | std::views::transform([](const AttrInfo& info) -> const std::string& {
            return info.name;
        });

        auto names = fmt::format("{}", fmt::join(fieldNames, ", "));
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
        for (auto& [type, info] : allowed | std::views::filter([](const auto& attr) {
            return attr.second.recommended;
        })) {
            if (!uniqueAttrs.contains(type)) {
                ctx.log<IDL_STATUS_W2001>(decl->location, decl->fullname(), info.name);
            }
        }
    }

    template <typename Attr>
    std::pair<std::type_index, AttrInfo> add(bool recommended = false) {
        return {
            typeid(Attr), { getName<Attr>(), recommended }
        };
    }

    template <typename Attr>
    std::string getName() {
        AttrName name(ctx);
        Attr attr{};
        attr.accept(name);
        return name.str;
    }
};

struct AttrDocValidatorRules : Visitor {
    explicit AttrDocValidatorRules(Context& ctx) noexcept : Visitor(ctx) {
    }

    void discarded(ASTNode* node) override {
        if (auto attr = node->as<ASTAttr>()) {
            if (attr->as<ASTDocAttr>()) {
                if (attr->as<ASTDocAttr>()->message.empty()) {
                    AttrName name(ctx);
                    attr->accept(name);
                    ctx.log<IDL_STATUS_E3016>(attr->location);
                }
            } else {
                AttrName name(ctx);
                attr->accept(name);
                ctx.log<IDL_STATUS_E3015>(attr->location, name.str);
            }
        }
    }
};

struct AttrIDocValidatorRules : Visitor {
    explicit AttrIDocValidatorRules(Context& ctx) noexcept : Visitor(ctx) {
    }

    void visit(ASTAttrDetail* node) override {
    }

    void discarded(ASTNode* node) {
        if (auto attr = node->as<ASTAttr>()) {
            ctx.log<IDL_STATUS_E3018>(attr->location);
        }
    }
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
        if (!args.empty()) {
            node->message = std::move(args);
        } else {
            ctx.log<IDL_STATUS_E3014>(node->location);
        }
    }

    void visit(ASTAttrDetail* node) override {
        if (!args.empty()) {
            node->message = std::move(args);
        } else {
            ctx.log<IDL_STATUS_E3017>(node->location);
        }
    }

    void visit(ASTAttrValue* node) override {
        if (!args.empty()) {
            std::type_index type = typeid(*args[0]);
            for (auto arg : args) {
                if (arg || arg->as<ASTLiteral>() || arg->as<ASTDeclRef>()) {
                    if (typeid(*arg) != type) {
                        ctx.log<IDL_STATUS_E3026>(node->location);
                        break;
                    }
                    node->values.push_back(arg);
                } else {
                    ctx.log<IDL_STATUS_E3025>(node->location);
                    break;
                }
            }
        } else {
            ctx.log<IDL_STATUS_E3024>(node->location);
        }
    }

    void visit(ASTAttrType* node) override {
        if (args.size() == 1 && args[0] && args[0]->as<ASTDeclRef>()) {
            node->type = args[0]->as<ASTDeclRef>();
        } else {
            ctx.log<IDL_STATUS_E3027>(node->location);
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
    HierarchyRules(Context& ctx, ASTDecl* lastDecl) noexcept : Visitor(ctx), lastDecl(lastDecl) {
    }

    void visit(ASTApi* node) override {
        if (ctx._api != nullptr) {
            ctx.log<IDL_STATUS_E3010>(node->location, node->fullname());
        } else {
            ctx._api = node;
        }
    }

    void visit(ASTImport* node) override {
        if (checkApi()) {
            auto [parent, childs] = findRoot();
            node->parent          = parent;
            childs->push_back(node);
        }
    }

    void visit(ASTEnum* node) override {
        if (checkApi()) {
            auto [parent, childs] = findRoot();
            node->parent          = parent;
            childs->push_back(node);
        }
    }

    void visit(ASTConst* node) override {
        if (auto parent = findParent<ASTEnum>()) {
            node->parent = parent;
            parent->consts.push_back(node);
        } else {
            ctx.log<IDL_STATUS_E3023>(node->location, node->name);
            node->parent = ctx.api();
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

    template <typename Node>
    Node* findParent() noexcept {
        auto node = lastDecl;
        while (node) {
            if (auto result = node->as<Node>()) {
                return result;
            }
            node = node->parent->as<ASTDecl>();
        }
        return nullptr;
    }

    template <typename... Node>
    ASTDecl* findAnyParent() noexcept {
        auto node = lastDecl;
        while (node) {
            if ((node->as<Node>() || ...)) {
                return node;
            }
            node = node->parent->as<ASTDecl>();
        }
        return nullptr;
    }

    std::pair<ASTDecl*, std::vector<ASTDecl*>*> findRoot() noexcept {
        auto parent = findAnyParent<ASTApi, ASTImport>();
        if (parent) {
            if (auto api = parent->as<ASTApi>()) {
                return { api, &api->decls };
            } else if (auto import = parent->as<ASTImport>()) {
                return { import, &import->decls };
            }
        }
        return {};
    }

    ASTDecl* lastDecl;
};

} // namespace idl

#endif
