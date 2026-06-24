#ifndef IDL_RULES_HPP
#define IDL_RULES_HPP

#include "visitors.hpp"

namespace idl {

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

    void visit(ASTApi* node) override {
        static std::map allowed = { add<ASTAttrVersion>(true), add<ASTAttrBrief>(true), add<ASTAttrDetail>(true),
                                    add<ASTAttrCName>(),       add<ASTAttrTokenizer>(), add<ASTAttrOrder>(),
                                    add<ASTAttrSingle>() };
        validate(node, allowed, node->attrs);
    }

    void visit(ASTImport* node) override {
        static std::map allowed = { add<ASTAttrBrief>(true), add<ASTAttrDetail>(true) };
        validate(node, allowed, node->attrs);
    }

    void visit(ASTEnum* node) override {
        static std::map allowed = { add<ASTAttrFlags>(),      add<ASTAttrHex>(),   add<ASTAttrBrief>(true),
                                    add<ASTAttrDetail>(true), add<ASTAttrCName>(), add<ASTAttrTokenizer>() };
        validate(node, allowed, node->attrs);
    }

    void visit(ASTConst* node) override {
        static std::map allowed = {
            add<ASTAttrDetail>(true), add<ASTAttrValue>(), add<ASTAttrCName>(), add<ASTAttrTokenizer>()
        };
        validate(node, allowed, node->attrs);
    }

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
            static const std::regex semver_regex(R"((0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*))");
            std::smatch matches;
            // Compatibility with old version format: "1.2.3" instead of "version(1, 2, 3)"
            if (std::regex_match(args[0]->as<ASTLiteralStr>()->value, matches, semver_regex)) {
                const auto major = std::stoll(matches[1].str());
                const auto minor = std::stoll(matches[2].str());
                const auto micro = std::stoll(matches[3].str());
                if (major <= 255 && minor <= 255 && micro <= 255) {
                    node->version = ASTAttrVersion::Semver{ (uint8_t) major, (uint8_t) minor, (uint8_t) micro };
                }
                return;
            }
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

    void visit(ASTAttrCName* node) override {
        if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralStr>()) {
            node->name = args[0]->as<ASTLiteralStr>()->value;
            if (std::any_of(node->name.begin(), node->name.end(), [](auto ch) {
                return !(std::isalpha(ch) || std::isdigit(ch) || ch == '_');
            })) {
                ctx.log<IDL_STATUS_E3029>(node->location, node->name);
            }
        } else {
            ctx.log<IDL_STATUS_E3028>(node->location);
        }
    }

    void visit(ASTAttrTokenizer* node) override {
        if (!args.empty()) {
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
        }
    }

    void visit(ASTAttrOrder* node) override {
        if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralBool>()) {
            node->autoOrder = args[0]->as<ASTLiteralBool>()->value;
        } else if (args.empty()) {
            node->autoOrder = true;
        } else {
            ctx.log<IDL_STATUS_E3019>(node->location);
        }
    }

    void visit(ASTAttrSingle* node) override {
        if (args.size() == 1 && args[0] && args[0]->as<ASTLiteralBool>()) {
            node->singleOutput = args[0]->as<ASTLiteralBool>()->value;
        } else if (args.empty()) {
            node->singleOutput = true;
        } else {
            ctx.log<IDL_STATUS_E3030>(node->location);
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
