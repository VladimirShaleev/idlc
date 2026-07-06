#ifndef IDL_RULES_HPP
#define IDL_RULES_HPP

#include "visitors.hpp"

namespace idl {

struct AttrValueOrTypeRules {
    explicit AttrValueOrTypeRules(Context& ctx) noexcept : ctx(ctx) {
    }

    void visit(ASTNode*, Tag<ASTNodeType::Const>) noexcept {
        isValue = true;
    }

    template <ASTNodeType Type>
    void visit(ASTNode* node, Tag<Type>) noexcept {
    }

    Context& ctx;
    bool isValue{};
};

struct AttrValidatorRules {
    struct AttrInfo {
        std::string name;
        bool recommended;
    };

    explicit AttrValidatorRules(Context& ctx) noexcept : ctx(ctx) {
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Api>) noexcept {
        static std::map allowed = { add<ASTNodeType::AttrVersion>(),          add<ASTNodeType::AttrDocBrief>(true),
                                    add<ASTNodeType::AttrDocDetail>(true),    add<ASTNodeType::AttrCName>(),
                                    add<ASTNodeType::AttrTokenizer>(),        add<ASTNodeType::AttrOrder>(),
                                    add<ASTNodeType::AttrSingle>(),           add<ASTNodeType::AttrDocAuthor>(true),
                                    add<ASTNodeType::AttrDocCopyright>(true), add<ASTNodeType::AttrDocLicense>(true) };
        validate(node, allowed);
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Import>) noexcept {
        static std::map allowed = { add<ASTNodeType::AttrDocBrief>(true), add<ASTNodeType::AttrDocDetail>(true) };
        validate(node, allowed);
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Enum>) noexcept {
        static std::map allowed = { add<ASTNodeType::AttrFlags>(),        add<ASTNodeType::AttrHex>(),
                                    add<ASTNodeType::AttrDocBrief>(true), add<ASTNodeType::AttrDocDetail>(true),
                                    add<ASTNodeType::AttrCName>(),        add<ASTNodeType::AttrTokenizer>() };
        validate(node, allowed);
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Const>) noexcept {
        static std::map allowed = { add<ASTNodeType::AttrDocDetail>(true),
                                    add<ASTNodeType::AttrValue>(),
                                    add<ASTNodeType::AttrCName>(),
                                    add<ASTNodeType::AttrTokenizer>() };
        validate(node, allowed);
    }

    template <ASTNodeType Type>
    void visit(ASTNode* node, Tag<Type>) noexcept {
        if (astNodeIs(node, ASTNodeType::Decl)) {
            const auto token = ctx.visit<DeclToken>(node).str;
            ctx.log<IDL_STATUS_E3006>(node->location, token, ctx.declFullname(node));
        } else {
            assert(!"attempt to validate attributes for a non-declaration node");
        }
    }

    void validate(ASTNode* node, const std::map<ASTNodeType, AttrInfo>& allowed) {
        auto fieldNames =
            allowed | std::views::values | std::views::transform([](const AttrInfo& info) -> const std::string& {
            return info.name;
        });

        auto names = fmt::format("{}", fmt::join(fieldNames, ", "));
        std::set<ASTNodeType> uniqueAttrs;
        auto curr = node->child;
        while (curr != NodeHandleNone) {
            auto attr = ctx.getNode(curr);
            curr      = attr->sibling;
            if (!astNodeIs(attr, ASTNodeType::Attr)) {
                continue;
            }
            if (!allowed.contains(attr->type)) {
                const auto name  = ctx.visit<AttrName>(attr).str;
                const auto token = ctx.visit<DeclToken>(node).str;
                ctx.log<IDL_STATUS_E3005>(attr->location, name, token, ctx.declFullname(node), names);
            }
            if (!uniqueAttrs.insert(attr->type).second) {
                const auto name  = ctx.visit<AttrName>(attr).str;
                const auto token = ctx.visit<DeclToken>(node).str;
                ctx.log<IDL_STATUS_E3007>(attr->location, name, token, ctx.declFullname(node));
            }
        }
        for (auto& [type, info] : allowed | std::views::filter([](const auto& attr) {
            return attr.second.recommended;
        })) {
            if (!uniqueAttrs.contains(type)) {
                ctx.log<IDL_STATUS_W2001>(node->location, ctx.declFullname(node), info.name);
            }
        }
    }

    template <ASTNodeType Type>
    std::pair<ASTNodeType, AttrInfo> add(bool recommended = false) {
        return {
            Type, { getName<Type>(), recommended }
        };
    }

    template <ASTNodeType Type>
    std::string getName() {
        ASTNode node{};
        node.type = Type;
        return ctx.visit<AttrName>(&node).str;
    }

    Context& ctx;
};

struct AttrDocValidatorRules {
    explicit AttrDocValidatorRules(Context& ctx) noexcept : ctx(ctx) {
    }

    template <ASTNodeType Type>
    void visit(ASTNode* node, Tag<Type>) noexcept {
        if (astNodeIs(node, ASTNodeType::Attr)) {
            if (astNodeIs(node, ASTNodeType::AttrDoc)) {
                if (node->child == NodeHandleNone) {
                    ctx.log<IDL_STATUS_E3016>(node->location);
                }
            } else {
                const auto name = ctx.visit<AttrName>(node).str;
                ctx.log<IDL_STATUS_E3015>(node->location, name);
            }
        }
    }

    Context& ctx;
};

struct AttrIDocValidatorRules {
    explicit AttrIDocValidatorRules(Context& ctx) noexcept : ctx(ctx) {
    }

    void visit(ASTNode*, Tag<ASTNodeType::AttrDocDetail>) noexcept {
    }

    template <ASTNodeType Type>
    void visit(ASTNode* node, Tag<Type>) noexcept {
        if (astNodeIs(node, ASTNodeType::Attr)) {
            ctx.log<IDL_STATUS_E3018>(node->location);
        }
    }

    Context& ctx;
};

struct AttrArgRules {
    AttrArgRules(Context& ctx, ASTNodeHandle argFrist, ASTNodeHandle argLast, size_t argCount) noexcept :
        ctx(ctx),
        argFrist(argFrist),
        argLast(argLast),
        argCount(argCount) {
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrVersion>) {
        auto arg0 = ctx.getNode(argFrist);
        auto arg1 = arg0 ? ctx.getNode(arg0->sibling) : nullptr;
        auto arg2 = arg1 ? ctx.getNode(arg1->sibling) : nullptr;
        if (argCount == 3 && arg0 && arg1 && arg2 && astNodeIs(arg0, ASTNodeType::LiteralInt) &&
            astNodeIs(arg1, ASTNodeType::LiteralInt) && astNodeIs(arg2, ASTNodeType::LiteralInt)) {
            const auto major = arg0->valueInt;
            const auto minor = arg1->valueInt;
            const auto micro = arg2->valueInt;
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
                // node->version.isSemver = true;
                // node->version.semver   = ASTVersion::Semver{ (uint8_t) major, (uint8_t) minor, (uint8_t) micro };
            }
        } else if (argCount == 1 && arg0 && astNodeIs(arg0, ASTNodeType::LiteralStr)) {
            static const std::regex semverRegex(R"((0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*))");
            std::smatch matches;
            // Compatibility with old version format: "1.2.3" instead of "version(1, 2, 3)"
            auto strView = ctx.getStr(arg0->valueStr);
            auto str     = std::string(strView.data(), strView.length());
            if (std::regex_match(str, matches, semverRegex)) {
                const auto major = std::stoll(matches[1].str());
                const auto minor = std::stoll(matches[2].str());
                const auto micro = std::stoll(matches[3].str());
                if (major <= 255 && minor <= 255 && micro <= 255) {
                    // node->version.isSemver = true;
                    // node->version.semver   = ASTVersion::Semver{ (uint8_t) major, (uint8_t) minor, (uint8_t) micro };
                }
                return;
            }
            // node->version.isSemver = false;
            // node->version.ver      = arg0->valueStr;
        } else {
            ctx.log<IDL_STATUS_E3003>(node->location);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrDocAuthor>) {
        static const std::regex email(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        // TODO:
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrDocBrief>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            ctx.log<IDL_STATUS_E3014>(node->location);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrDocDetail>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            ctx.log<IDL_STATUS_E3017>(node->location);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrDocCopyright>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            ctx.log<IDL_STATUS_E3034>(node->location);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrDocLicense>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            ctx.log<IDL_STATUS_E3035>(node->location);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrValue>) {
        if (argCount > 0) {
            const auto type = ctx.getNode(argFrist)->type;
            node->child     = argFrist;
            for (auto arg : ctx.getNodeChilds(node)) {
                if (arg != NodeHandleNone || astNodeIs(node, ASTNodeType::Literal) ||
                    astNodeIs(node, ASTNodeType::DeclRef)) {
                    if (ctx.getNode(arg)->type != type) {
                        ctx.log<IDL_STATUS_E3026>(node->location);
                        node->child = NodeHandleNone;
                        break;
                    }
                } else {
                    ctx.log<IDL_STATUS_E3025>(node->location);
                    node->child = NodeHandleNone;
                    break;
                }
            }
        } else {
            ctx.log<IDL_STATUS_E3024>(node->location);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrType>) {
        // if (args.size() == 1 && args[0] && args[0]->as<ASTDeclRef>()) {
        //     // node->type = args[0]->as<ASTDeclRef>();
        // } else {
        //     ctx.log<IDL_STATUS_E3027>(node->location);
        // }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrCName>) {
        // if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralStr>()) {
        //     node->name = args[0]->as<ASTLiteralStr>()->value;
        //     if (std::any_of(node->name.begin(), node->name.end(), [](auto ch) {
        //         return !(std::isalpha(ch) || std::isdigit(ch) || ch == '_');
        //     })) {
        //         ctx.log<IDL_STATUS_E3029>(node->location, node->name);
        //     }
        // } else {
        //     ctx.log<IDL_STATUS_E3028>(node->location);
        // }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrTokenizer>) {
        /* if (!args.empty()) {
             auto isAllIntegers = std::all_of(args.begin(), args.end(), [](auto arg) {
                 return arg && arg->as<ASTLiteralInt>();
             });
             if (isAllIntegers) {
                 for (auto arg : args) {
                     node->nums.push_back((int) arg->as<ASTLiteralInt>()->value);
                 }
             } else if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralStr>()) {
                 static std::regex pattern(R"(\^?\d+(-\^?\d+)*)");
                 // Compatibility with old tokenizer format: "1-^2-3" instead of "tokenizer(1, -2, 3)"
                 if (std::regex_match(args[0]->as<ASTLiteralStr>()->value, pattern)) {
                     std::stringstream ss(args[0]->as<ASTLiteralStr>()->value);
                     std::string token;
                     while (std::getline(ss, token, '-')) {
                         if (token[0] == '^') {
                             node->nums.push_back(-std::stoi(token.substr(1)));
                         } else {
                             node->nums.push_back(std::stoi(token));
                         }
                     }
                 } else {
                     ctx.log<IDL_STATUS_E3031>(node->location, args[0]->as<ASTLiteralStr>()->value);
                 }
             } else {
                 ctx.log<IDL_STATUS_E3032>(node->location);
             }
         } else {
             ctx.log<IDL_STATUS_E3033>(node->location);
         }*/
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrOrder>) {
        // if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralBool>()) {
        //     node->autoOrder = args[0]->as<ASTLiteralBool>()->value;
        // } else if (args.empty()) {
        //     node->autoOrder = true;
        // } else {
        //     ctx.log<IDL_STATUS_E3019>(node->location);
        // }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::AttrSingle>) {
        // if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralBool>()) {
        //     node->singleOutput = args[0]->as<ASTLiteralBool>()->value;
        // } else if (args.empty()) {
        //     node->singleOutput = true;
        // } else {
        //     ctx.log<IDL_STATUS_E3030>(node->location);
        // }
    }

    template <ASTNodeType Type>
    void visit(ASTNode* node, Tag<Type>) noexcept {
        int i = 5;
    }

    Context& ctx;
    ASTNodeHandle argFrist;
    ASTNodeHandle argLast;
    size_t argCount;
};

struct HierarchyRules {
    HierarchyRules(Context& ctx, ASTNodeHandle lastNode, ASTNodeHandle currNode) noexcept :
        ctx(ctx),
        lastNode(lastNode),
        currNode(currNode) {
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Api>) {
        if (ctx.api() != NodeHandleNone) {
            ctx.log<IDL_STATUS_E3010>(node->location, ctx.declFullname(node));
        } else {
            // forEachAttributes(nodePtr);
            ctx.initBuiltins(currNode);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Import>) {
        if (checkApi()) {
            auto parent  = findRoot();
            node->parent = parent;
            ctx.addChild(parent, currNode);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Enum>) {
        if (checkApi()) {
            auto parent  = findRoot();
            node->parent = parent;
            ctx.addChild(parent, currNode);
            // forEachAttributes(node);
        }
    }

    void visit(ASTNode* node, Tag<ASTNodeType::Const>) {
        if (auto parent = findParent<ASTNodeType::Enum>(); parent != NodeHandleNone) {
            node->parent = parent;
            ctx.addChild(parent, currNode);
            // forEachAttributes(node);
        } else {
            ctx.log<IDL_STATUS_E3023>(node->location, ctx.declFullname(node));
            node->parent = ctx.api();
        }
    }

    /*
        void visit(ASTAttrBrief* node) override {
            forEachDocs(node);
        }

        void visit(ASTAttrDetail* node) override {
            forEachDocs(node);
        }

        void visit(ASTAttrCopyright* node) override {
            forEachDocs(node);
        }

        void visit(ASTAttrLicense* node) override {
            forEachDocs(node);
        }

        void visit(ASTAttrVersion* node) override {
            node->parent = lastNode;
        }

        void visit(ASTAttrCName* node) override {
            node->parent = lastNode;
        }

        void visit(ASTAttrOrder* node) override {
            node->parent = lastNode;
        }

        void visit(ASTAttrSingle* node) override {
            node->parent = lastNode;
        }

        void visit(ASTAttrHex* node) override {
            node->parent = lastNode;
        }

        void visit(ASTAttrValue* node) override {
            node->parent = lastNode;
            forEachChidls(node, node->values.begin(), node->values.end());
        }

        void visit(ASTDeclRef* node) override {
            node->parent = lastNode;
        }
    */

    template <ASTNodeType Type>
    void visit(ASTNode* node, Tag<Type>) noexcept {
        int i = 5;
    }

    bool checkApi() {
        if (ctx.api() == NodeHandleNone) {
            ctx.log<IDL_STATUS_E3011>(ASTLocation());
            return false;
        }
        return true;
    }

    /*
        template <typename It>
        void forEachChidls(ASTNode* parent, It begin, It end) noexcept {
            for (auto it = begin; it != end; ++it) {
                if (!(*it)->as<ASTLiteral>()) {
                    ctx.visit<HierarchyRules>(*it, parent);
                }
            }
        }

        void forEachAttributes(ASTDecl* parent) noexcept {
            // forEachChidls(parent, parent->attrs.begin(), parent->attrs.end());
        }

        void forEachDocs(ASTDocAttr* parent) noexcept {
            parent->parent = lastNode;
            // forEachChidls(parent, parent->message.begin(), parent->message.end());
        }
    */
    template <ASTNodeType... Types>
    ASTNodeHandle findParent() noexcept {
        auto curr = lastNode;
        auto node = ctx.getNode(curr);
        while (astNodeIs(node, ASTNodeType::Decl)) {
            if ((astNodeIs(node, Types) || ...)) {
                return curr;
            }
            curr = node->parent;
            node = ctx.getNode(curr);
        }
        return NodeHandleNone;
    }

    ASTNodeHandle findRoot() noexcept {
        return findParent<ASTNodeType::Api, ASTNodeType::Import>();
    }

    Context& ctx;
    ASTNodeHandle lastNode;
    ASTNodeHandle currNode;
};

//// struct BuildRules : Visitor {
////     explicit BuildRules(Context& ctx) noexcept : Visitor(ctx) {
////     }
////
////     explicit BuildRules(Context& ctx, ASTConst* prevConst) noexcept : Visitor(ctx), prevConst(prevConst) {
////     }
////
////     void visit(ASTApi* node) override {
////         buildDoc(node);
////         for (auto decl : node->decls) {
////             ctx.visit<BuildRules>(decl);
////         }
////     }
////
////     void visit(ASTEnum* node) override {
////         buildDoc(node);
////         ASTConst* prev = nullptr;
////         for (auto c : node->consts) {
////             ctx.visit<BuildRules>(c, prev);
////             prev = c;
////         }
////     }
////
////     void visit(ASTConst* node) override {
////         auto attrValue = node->findAttr<ASTAttrValue>();
////
////         if (!attrValue) {
////             attrValue         = ctx.allocNode<ASTAttrValue>(node->location);
////             attrValue->parent = node;
////             attrValue->values.push_back(ctx.addLiteral(node->location, prevConst ? prevConst->value + 1 : 0));
////             node->attrs.push_back(attrValue);
////         }
////
////         buildDoc(node);
////
////         int32_t evaluatedValue = 0;
////         for (auto value : attrValue->values) {
////             if (auto literal = value->as<ASTLiteralInt>()) {
////                 evaluatedValue |= literal->value;
////                 continue;
////             }
////
////             auto ref = value->as<ASTDeclRef>();
////             if (!ref) {
////                 // ctx.log<IDL_STATUS_E3041>(value->location);
////                 continue;
////             }
////
////             auto decl = ctx.resolveRef(node->parent->as<ASTDecl>(), ref->location, ref);
////             if (!decl) {
////                 continue;
////             }
////
////             auto refConst = decl->as<ASTConst>();
////             if (!refConst) {
////                 // ctx.log<IDL_STATUS_E3040>(ref->location, ref->name);
////                 continue;
////             }
////
////             if (!refConst->evaluated) {
////                 // ctx.log<IDL_STATUS_E3039>(ref->location, ref->name);
////                 continue;
////             }
////
////             evaluatedValue |= refConst->value;
////         }
////         node->value     = evaluatedValue;
////         node->evaluated = true;
////     }
////
////     void discarded(ASTNode* node) override {
////     }
////
////     void buildDoc(ASTDecl* decl) noexcept {
////         auto scope = decl->parent->as<ASTDecl>();
////         for (auto attr : decl->attrs) {
////             if (auto doc = attr->as<ASTDocAttr>()) {
////                 for (auto message : doc->message) {
////                     if (auto ref = message->as<ASTDeclRef>()) {
////                         ctx.resolveRef(scope, ref->location, ref);
////                     }
////                 }
////             }
////         }
////     }
////
////     ASTConst* prevConst{};
//// };
//
// struct BuildRules : Visitor {
//    explicit BuildRules(Context& ctx) noexcept : Visitor(ctx) {
//    }
//
//    void discarded(ASTNode* node) override {
//    }
//
//    void visit(ASTEnum* node) override {
//    }
//
//    void visit(ASTConst* node) override {
//        if (prevConst && prevConst->parent != node->parent) {
//            prevConst = nullptr;
//        }
//
//        auto attrValue = node->findAttr<ASTAttrValue>();
//        if (!attrValue) {
//            attrValue         = ctx.allocNode<ASTAttrValue>(node->location);
//            attrValue->parent = node;
//            attrValue->values.push_back(ctx.addLiteral(node->location, prevConst ? prevConst->value + 1 : 0));
//            node->childs.push_back(attrValue);
//        }
//
//        int32_t evaluatedValue = 0;
//        for (auto value : attrValue->values) {
//            if (auto literal = value->as<ASTLiteralInt>()) {
//                evaluatedValue |= literal->value;
//                continue;
//            }
//
//            auto ref = value->as<ASTDeclRef>();
//            if (!ref) {
//                // ctx.log<IDL_STATUS_E3041>(value->location);
//                continue;
//            }
//
//            auto decl = ctx.resolveRef(node->parent->as<ASTDecl>(), ref->location, ref);
//            if (!decl) {
//                continue;
//            }
//
//            auto refConst = decl->as<ASTConst>();
//            if (!refConst) {
//                // ctx.log<IDL_STATUS_E3040>(ref->location, ref->name);
//                continue;
//            }
//
//            if (!refConst->evaluated) {
//                // ctx.log<IDL_STATUS_E3039>(ref->location, ref->name);
//                continue;
//            }
//
//            evaluatedValue |= refConst->value;
//        }
//        node->value     = evaluatedValue;
//        node->evaluated = true;
//
//        prevConst = node;
//    }
//
//    void visit(ASTDeclRef* node) override {
//        auto parent = findParent(node);
//        // auto parentDecl = parent->parent ? parent->parent->as<ASTDecl>() : nullptr;
//        // ctx.resolveRef(parentDecl, node->location, node);
//    }
//
//    ASTDecl* findParent(ASTNode* node) noexcept {
//        while (node) {
//            if (auto decl = node->as<ASTDecl>()) {
//                return decl;
//            }
//            node = node->parent;
//        }
//        return nullptr;
//    }
//
//    ASTConst* prevConst{};
//};

} // namespace idl

#endif
