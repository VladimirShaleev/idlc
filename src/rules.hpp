#ifndef IDL_RULES_HPP
#define IDL_RULES_HPP

#include "visitors.hpp"

namespace idl {

struct AttrValueOrTypeRules {
    void visit(ASTNodeRef&, Tag<ASTNodeType::Const>) noexcept {
        isValue = true;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    bool isValue{};
};

struct AttrValidatorRules {
    struct AttrInfo {
        std::string name;
        bool recommended;
    };

    AttrValidatorRules(Context& ctx) noexcept : ctx(ctx) {
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Api>) noexcept {
        static std::map allowed = { add<ASTNodeType::AttrVersion>(),          add<ASTNodeType::AttrDocBrief>(true),
                                    add<ASTNodeType::AttrDocDetail>(true),    add<ASTNodeType::AttrCName>(),
                                    add<ASTNodeType::AttrTokenizer>(),        add<ASTNodeType::AttrOrder>(),
                                    add<ASTNodeType::AttrSingle>(),           add<ASTNodeType::AttrDocAuthor>(true),
                                    add<ASTNodeType::AttrDocCopyright>(true), add<ASTNodeType::AttrDocLicense>(true) };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Import>) noexcept {
        static std::map allowed = { add<ASTNodeType::AttrDocBrief>(true), add<ASTNodeType::AttrDocDetail>(true) };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Enum>) noexcept {
        static std::map allowed = { add<ASTNodeType::AttrFlags>(),        add<ASTNodeType::AttrHex>(),
                                    add<ASTNodeType::AttrDocBrief>(true), add<ASTNodeType::AttrDocDetail>(true),
                                    add<ASTNodeType::AttrCName>(),        add<ASTNodeType::AttrTokenizer>() };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Const>) noexcept {
        static std::map allowed = { add<ASTNodeType::AttrDocDetail>(true),
                                    add<ASTNodeType::AttrValue>(),
                                    add<ASTNodeType::AttrCName>(),
                                    add<ASTNodeType::AttrTokenizer>() };
        validate(node, allowed);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        if (node.is<ASTNodeType::Decl>()) {
            const auto token = node.accept<DeclToken>().str;
            node.ctx().log<IDL_STATUS_E3006>(node->location, token, node.fullname());
        } else {
            assert(!"attempt to validate attributes for a non-declaration node");
        }
    }

    void validate(ASTNodeRef& node, const std::map<ASTNodeType, AttrInfo>& allowed) {
        auto fieldNames =
            allowed | std::views::values | std::views::transform([](const AttrInfo& info) -> const std::string& {
            return info.name;
        });

        auto names = fmt::format("{}", fmt::join(fieldNames, ", "));
        std::set<ASTNodeType> uniqueAttrs;
        for (auto attr : node.attrs()) {
            if (!allowed.contains(attr->type)) {
                const auto name  = attr.accept<AttrName>().str;
                const auto token = node.accept<DeclToken>().str;
                attr.ctx().log<IDL_STATUS_E3005>(attr->location, name, token, node.fullname(), names);
            }
            if (!uniqueAttrs.insert(attr->type).second) {
                const auto name  = attr.accept<AttrName>().str;
                const auto token = node.accept<DeclToken>().str;
                attr.ctx().log<IDL_STATUS_E3007>(attr->location, name, token, node.fullname());
            }
        }
        for (auto& [type, info] : allowed | std::views::filter([](const auto& attr) {
            return attr.second.recommended;
        })) {
            if (!uniqueAttrs.contains(type)) {
                node.ctx().log<IDL_STATUS_W2001>(node->location, node.fullname(), info.name);
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
        return ASTNodeRef::byType<Type>(ctx).accept<AttrName>().str;
    }

    Context& ctx;
};

struct AttrDocValidatorRules {
    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        if (node.is<ASTNodeType::Attr>()) {
            if (node.is<ASTNodeType::AttrDoc>()) {
                if (!node.hasChilds()) {
                    node.ctx().log<IDL_STATUS_E3016>(node->location);
                }
            } else {
                const auto name = node.accept<AttrName>().str;
                node.ctx().log<IDL_STATUS_E3015>(node->location, name);
            }
        }
    }
};

struct AttrIDocValidatorRules {
    void visit(ASTNodeRef&, Tag<ASTNodeType::AttrDocDetail>) noexcept {
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        if (node.is<ASTNodeType::Attr>()) {
            node.ctx().log<IDL_STATUS_E3018>(node->location);
        }
    }
};

struct AttrArgRules {
    AttrArgRules(ASTNodeHandle argFrist, ASTNodeHandle argLast, size_t argCount) noexcept :
        argFrist(argFrist),
        argLast(argLast),
        argCount(argCount) {
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrVersion>) {
        auto& ctx = node.ctx();
        auto arg0 = ctx.getNodeRef(argFrist);
        auto arg1 = arg0 ? ctx.getNodeRef(arg0->sibling) : ASTNodeRef(ctx);
        auto arg2 = arg1 ? ctx.getNodeRef(arg1->sibling) : ASTNodeRef(ctx);
        if (argCount == 3 && arg0.is<ASTNodeType::LiteralInt>() && arg1.is<ASTNodeType::LiteralInt>() &&
            arg2.is<ASTNodeType::LiteralInt>()) {
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
                node->child = argFrist;
            }
        } else if (argCount == 1 && arg0.is<ASTNodeType::LiteralStr>()) {
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
                    static const std::string filename = "<runtime>";

                    const auto loc = location(position(&filename, 1, 1));
                    auto nodeMajor = ctx.allocNode(loc, ASTNodeType::LiteralInt);
                    auto nodeMinor = ctx.allocNode(loc, ASTNodeType::LiteralInt);
                    auto nodeMicro = ctx.allocNode(loc, ASTNodeType::LiteralInt);

                    ctx.getNode(nodeMajor)->valueInt = major;
                    ctx.getNode(nodeMinor)->valueInt = minor;
                    ctx.getNode(nodeMicro)->valueInt = micro;
                    ctx.getNode(nodeMajor)->sibling  = nodeMinor;
                    ctx.getNode(nodeMinor)->sibling  = nodeMicro;

                    node->child = nodeMajor;
                    arg0->type  = ASTNodeType::Tombstone;
                    return;
                }
            }
            node->child = argFrist;
        } else {
            ctx.log<IDL_STATUS_E3003>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrDocAuthor>) {
        static const std::regex email(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        // TODO:
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrDocBrief>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrDocDetail>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3017>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrDocCopyright>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3034>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrDocLicense>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3035>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrValue>) {
        if (argCount > 0) {
            auto arg0       = node.ctx().getNodeRef(argFrist);
            const auto type = arg0 ? arg0->type : ASTNodeType::Tombstone;
            node->child     = argFrist;
            for (auto child : node) {
                if (child.is<ASTNodeType::Literal, ASTNodeType::DeclRef>()) {
                    if (child->type != type) {
                        node.ctx().log<IDL_STATUS_E3026>(node->location);
                        node->child = NodeHandleNone;
                        break;
                    }
                } else {
                    node.ctx().log<IDL_STATUS_E3025>(node->location);
                    node->child = NodeHandleNone;
                    break;
                }
            }
        } else {
            node.ctx().log<IDL_STATUS_E3024>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrType>) {
        auto arg0 = node.ctx().getNodeRef(argFrist);
        if (argCount == 1 && arg0.is<ASTNodeType::DeclRef>()) {
            // node->type = args[0]->as<ASTDeclRef>();
        } else {
            node.ctx().log<IDL_STATUS_E3027>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrCName>) {
        auto& ctx = node.ctx();
        auto arg0 = ctx.getNodeRef(argFrist);
        if (argCount == 1 && arg0.is<ASTNodeType::LiteralStr>()) {
            auto name = arg0.valueStr();
            if (std::any_of(name.begin(), name.end(), [](auto ch) {
                return !(std::isalpha(ch) || std::isdigit(ch) || ch == '_');
            })) {
                ctx.log<IDL_STATUS_E3029>(node->location, name);
            } else {
                node->child = argFrist;
            }
        } else {
            ctx.log<IDL_STATUS_E3028>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrTokenizer>) {
        auto& ctx = node.ctx();
        if (argCount > 0) {
            node->child = argFrist;

            auto isAllIntegers = std::all_of(node.begin(), node.end(), [this](auto arg) {
                return arg.is<ASTNodeType::LiteralInt>();
            });
            if (!isAllIntegers) {
                auto arg0 = ctx.getNodeRef(argFrist);
                if (argCount == 1 && arg0.is<ASTNodeType::LiteralStr>()) {
                    static std::regex pattern(R"(\^?\d+(-\^?\d+)*)");
                    auto strView = arg0.valueStr();
                    auto str     = std::string(strView.data(), strView.length());
                    // Compatibility with old tokenizer format: "1-^2-3" instead of "tokenizer(1, -2, 3)"
                    if (std::regex_match(str, pattern)) {
                        std::stringstream ss(str);
                        std::string token;
                        ASTNode* lastNum = nullptr;
                        while (std::getline(ss, token, '-')) {
                            static const std::string filename = "<runtime>";

                            const auto loc = location(position(&filename, 1, 1));

                            auto num = ctx.allocNode(loc, ASTNodeType::LiteralInt);
                            if (token[0] == '^') {
                                ctx.getNode(num)->valueInt = -std::stoi(token.substr(1));
                            } else {
                                ctx.getNode(num)->valueInt = std::stoi(token);
                            }
                            if (!lastNum) {
                                node->child = num;
                            } else {
                                lastNum->sibling = num;
                            }
                            lastNum = ctx.getNode(num);
                        }
                    } else {
                        ctx.log<IDL_STATUS_E3031>(node->location, str);
                        node->child = NodeHandleNone;
                    }
                } else {
                    ctx.log<IDL_STATUS_E3032>(node->location);
                    node->child = NodeHandleNone;
                }
                if (arg0) {
                    arg0->type = ASTNodeType::Tombstone;
                }
            }
        } else {
            ctx.log<IDL_STATUS_E3033>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrOrder>) {
        auto& ctx = node.ctx();
        auto arg0 = ctx.getNodeRef(argFrist);
        if (argCount == 1 && arg0.is<ASTNodeType::LiteralBool>()) {
            node->child = argFrist;
        } else if (argCount == 0) {
            static const std::string filename = "<runtime>";

            const auto loc = location(position(&filename, 1, 1));
            auto valueBool = ctx.allocNode(loc, ASTNodeType::LiteralBool);

            ctx.getNode(valueBool)->valueBool = true;

            node->child = valueBool;
        } else {
            ctx.log<IDL_STATUS_E3019>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::AttrSingle>) {
        auto& ctx = node.ctx();
        auto arg0 = ctx.getNodeRef(argFrist);
        if (argCount == 1 && arg0.is<ASTNodeType::LiteralBool>()) {
            node->child = argFrist;
        } else if (argCount == 0) {
            static const std::string filename = "<runtime>";

            const auto loc = location(position(&filename, 1, 1));
            auto valueBool = ctx.allocNode(loc, ASTNodeType::LiteralBool);

            ctx.getNode(valueBool)->valueBool = true;

            node->child = valueBool;
        } else {
            node.ctx().log<IDL_STATUS_E3030>(node->location);
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        if (argCount > 0) {
            const auto name = node.accept<AttrName>().str;
            node.ctx().log<IDL_STATUS_E3008>(node->location, name);
        }
    }

    ASTNodeHandle argFrist;
    ASTNodeHandle argLast;
    size_t argCount;
};

struct HierarchyRules {
    HierarchyRules(ASTNodeHandle lastNode) noexcept : lastNode(lastNode) {
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Api>) {
        auto& ctx = node.ctx();
        if (ctx.api()) {
            ctx.log<IDL_STATUS_E3010>(node->location, node.fullname());
        } else {
            ctx.initBuiltins(node);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Import>) {
        if (checkApi(node.ctx())) {
            auto parent  = findRoot(node.ctx());
            node->parent = parent.handle();
            parent.addChild(node);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Enum>) {
        if (checkApi(node.ctx())) {
            auto parent  = findRoot(node.ctx());
            node->parent = parent.handle();
            parent.addChild(node);
        }
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Const>) {
        if (auto parent = findParent<ASTNodeType::Enum>(node.ctx())) {
            node->parent = parent.handle();
            parent.addChild(node);
        } else {
            node.ctx().log<IDL_STATUS_E3023>(node->location, node.fullname());
            node->parent = node.ctx().api().handle();
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"no hierarchy rule is defined for this node type.");
    }

    static bool checkApi(Context& ctx) {
        if (!ctx.api()) {
            ctx.log<IDL_STATUS_E3011>(ASTLocation());
            return false;
        }
        return true;
    }

    template <ASTNodeType... Types>
    ASTNodeRef findParent(Context& ctx) noexcept {
        auto curr = ctx.getNodeRef(lastNode);
        while (curr.is<ASTNodeType::Decl>()) {
            if (curr.is<Types...>()) {
                return curr;
            }
            curr = curr.parent();
        }
        return ASTNodeRef(ctx);
    }

    ASTNodeRef findRoot(Context& ctx) noexcept {
        return findParent<ASTNodeType::Api, ASTNodeType::Import>(ctx);
    }

    ASTNodeHandle lastNode;
};

struct BuildRules {
    BuildRules(ASTNodeRef& prevConst, bool& prevE3041) noexcept : prevConst(prevConst), prevE3041(prevE3041) {
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::Const>) {
        auto& ctx = node.ctx();
        if (prevConst && prevConst.parent() != node.parent()) {
            prevConst = ASTNodeRef(ctx);
            prevE3041 = false;
        }

        auto attrValue = node.findChild<ASTNodeType::AttrValue>();
        if (attrValue) {
            for (auto value : attrValue) {
                if (value.is<ASTNodeType::LiteralInt>()) {
                    // add check range
                    continue;
                }

                if (!value.is<ASTNodeType::DeclRef>()) {
                    ctx.log<IDL_STATUS_E3040>(value->location);
                    node->flags |= ASTNODE_BUILD_ERROR;
                    continue;
                }

                auto decl = value.resolveRef(node.parent());
                if (!decl) {
                    node->flags |= ASTNODE_BUILD_ERROR;
                    continue;
                }

                if (!decl.is<ASTNodeType::Const>()) {
                    ctx.log<IDL_STATUS_E3038>(node->location);
                    node->flags |= ASTNODE_BUILD_ERROR;
                    continue;
                }

                if (decl == node) {
                    ctx.log<IDL_STATUS_E3039>(node->location, decl.fullname());
                    node->flags |= ASTNODE_BUILD_ERROR;
                    continue;
                }

                if (decl.buildError()) {
                    if (!prevE3041) {
                        prevE3041 = true;
                        ctx.log<IDL_STATUS_E3041>(node->location, node.fullname());
                    }
                    node->flags |= ASTNODE_BUILD_ERROR;
                    continue;
                }

                if (!decl.evaulated()) {
                    const auto [hasCyclic, deps] = findCyclicRelationshipConsts(node, decl);
                    if (hasCyclic) {
                        ctx.log<IDL_STATUS_E3042>(node->location, deps);
                        node->flags |= ASTNODE_BUILD_ERROR;
                    } else {
                        ctx.log<IDL_STATUS_W2003>(node->location, node.fullname(), decl.fullname());
                        node->flags |= ASTNODE_FORWARD_DECL;
                    }
                    continue;
                }
            }
        } else {
            static const std::string filename = "<runtime>";

            const auto loc = location(position(&filename, 1, 1));

            auto handle       = ctx.allocNode(loc, ASTNodeType::AttrValue);
            attrValue         = ASTNodeRef(ctx, handle);
            attrValue->parent = node.handle();
            attrValue->child  = ctx.allocNode(loc, ASTNodeType::LiteralInt);

            if (prevConst) {
                if (prevConst.buildError()) {
                    if (!prevE3041) {
                        prevE3041 = true;
                        ctx.log<IDL_STATUS_E3041>(node->location, node.fullname());
                    }
                    ctx.getNode(attrValue->child)->valueInt = 0;
                    node->flags |= ASTNODE_BUILD_ERROR;
                } else {
                    ctx.getNode(attrValue->child)->valueInt = calcConstDeps(prevConst) + 1;
                }
            } else {
                ctx.getNode(attrValue->child)->valueInt = 0;
            }
            node.addChild(attrValue);
        }

        if (!node.buildError()) {
            prevE3041 = false;
        }

        node->flags |= ASTNODE_EVAULATED;

        prevConst = node;
    }

    void visit(ASTNodeRef& node, Tag<ASTNodeType::DeclRef>) {
        auto parent       = findParent(node);
        auto parentParent = parent.parent();
        node.resolveRef(parentParent);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    uint64_t calcConstDeps(ASTNodeRef node) {
        if (!node.evaulated() || node.buildError()) {
            return 0;
        }
        uint64_t evaulated = 0;
        for (auto val : node.findChild<ASTNodeType::AttrValue>()) {
            if (val.is<ASTNodeType::LiteralInt>()) {
                evaulated |= val->valueInt;
            } else if (val.is<ASTNodeType::DeclRef>()) {
                auto decl = val.resolveRef(val.parent());
                if (!decl.is<ASTNodeType::Const>()) {
                    return 0;
                }
                evaulated |= calcConstDeps(decl);
            } else {
                return 0;
            }
        }
        return evaulated;
    }

    std::pair<bool, std::string> findCyclicRelationshipConsts(ASTNodeRef& target, ASTNodeRef next) {
        std::stack<ASTNodeRef> consts;
        consts.push(next);
        std::vector<ASTNodeRef> buffer;
        buffer.reserve(50);
        std::unordered_map<uint16_t, ASTNodeRef> cameFrom;
        cameFrom[next.handle().handle] = target;
        while (!consts.empty()) {
            auto top = consts.top();
            consts.pop();

            if (top == target) {
                buffer.clear();
                ASTNodeRef curr = target;
                do {
                    buffer.push_back(curr);
                    curr = cameFrom[curr.handle().handle];
                } while (curr != target);
                buffer.push_back(target);

                std::ostringstream ss;
                for (auto dep = buffer.rbegin(); dep != buffer.rend(); ++dep) {
                    if (dep != buffer.rbegin()) {
                        ss << " -> ";
                    }
                    ss << dep->fullname();
                }
                return { true, ss.str() };
            }

            auto declRefs = top.findChild<ASTNodeType::AttrValue>() | std::views::filter([this](const auto& value) {
                return value.is<ASTNodeType::DeclRef>();
            });
            buffer.clear();
            for (auto ref : declRefs) {
                if (auto decl = ref.resolveRef(findParent(ref)); decl.is<ASTNodeType::Const>()) {
                    if (!cameFrom.contains(decl.handle().handle)) {
                        cameFrom[decl.handle().handle] = top;
                    } else if (decl == target) {
                        cameFrom[decl.handle().handle] = top;
                    }
                    buffer.push_back(decl);
                }
            }
            for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
                consts.push(*it);
            }
        }
        return { false, "" };
    }

    ASTNodeRef findParent(ASTNodeRef node) noexcept {
        node = node.parent();
        while (node) {
            if (node.is<ASTNodeType::Decl>()) {
                return node;
            }
            node = node.parent();
        }
        return ASTNodeRef(node.ctx());
    }

    ASTNodeRef& prevConst;
    bool& prevE3041;
};

} // namespace idl

#endif
