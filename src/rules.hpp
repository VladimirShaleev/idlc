#ifndef IDL_RULES_HPP
#define IDL_RULES_HPP

#include "visitors.hpp"

namespace idl {

struct AttrValidatorRules {
    struct AttrInfo {
        std::string name;
        bool recommended;
    };

    AttrValidatorRules(Context& ctx) noexcept : ctx(ctx), options(options) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        static std::map allowed = {
            add<IDL_AST_NODE_TYPE_ATTR_VERSION>(),        add<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(true),
            add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true), add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),
            add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),      add<IDL_AST_NODE_TYPE_ATTR_SINGLE>(),
            add<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>(true), add<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>(true),
            add<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>(true)
        };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true) };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        static std::map allowed = {
            add<IDL_AST_NODE_TYPE_ATTR_FLAGS>(),         add<IDL_AST_NODE_TYPE_ATTR_HEX>(),
            add<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(true), add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true),
            add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),         add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),
            add<IDL_AST_NODE_TYPE_ATTR_TYPE>()
        };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_VALUE>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>() };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_STRUCT>) {
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TYPE>() };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FIELD>) {
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TYPE>(true) };
        validate(node, allowed);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        if (node.is<IDL_AST_NODE_TYPE_DECL>()) {
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
            if (!allowed.contains(attr.type())) {
                const auto name  = attr.accept<AttrName>().str;
                const auto token = node.accept<DeclToken>().str;
                attr.ctx().log<IDL_STATUS_E3005>(attr->location, name, token, node.fullname(), names);
            }
            if (!uniqueAttrs.insert(attr.type()).second) {
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
    const Options* options;
};

struct AttrDocValidatorRules {
    explicit AttrDocValidatorRules(bool multiline = false) noexcept : multiline(multiline) {
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) {
        if (node.is<IDL_AST_NODE_TYPE_ATTR>()) {
            if (node.is<IDL_AST_NODE_TYPE_ATTR_DOC>()) {
                if (!node.hasChilds()) {
                    node.ctx().log<IDL_STATUS_E3016>(node->location);
                } else if (multiline) {
                    node.setMultilineDoc();
                    auto minIndents = std::numeric_limits<size_t>::max();
                    auto isNewLine  = true;
                    for (auto arg : node) {
                        if (arg.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                            auto str = arg.valueStr();
                            if (isNewLine) {
                                if (str.length() && str[0] == ' ') {
                                    minIndents = std::min(minIndents, str.length());
                                }
                                isNewLine = false;
                            } else if (str.length() == 1 && str[0] == '\n') {
                                isNewLine = true;
                            }
                        }
                    }
                    isNewLine    = true;
                    auto prevArg = node.ctx().emptyNodeRef();
                    for (auto arg : node) {
                        if (arg.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                            auto str = arg.valueStr();
                            if (isNewLine) {
                                if (str.length() && str[0] == ' ') {
                                    auto substr = str.substr(minIndents);
                                    if (substr.empty()) {
                                        if (prevArg) {
                                            prevArg->sibling = arg->sibling;
                                        } else {
                                            node->child = arg->sibling;
                                        }
                                        arg->parent = HandleNone;
                                        arg->child  = HandleNone;
                                        continue;
                                    } else {
                                        arg->valueStr = node.result()->intern(substr);
                                    }
                                }
                                isNewLine = false;
                            } else if (str.length() == 1 && str[0] == '\n') {
                                isNewLine = true;
                            }
                        }
                        prevArg = arg;
                    }
                }
            } else {
                const auto name = node.accept<AttrName>().str;
                node.ctx().log<IDL_STATUS_E3015>(node->location, name);
            }
        }
    }

    bool multiline{};
};

struct AttrIDocValidatorRules {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>) noexcept {
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) {
        if (node.is<IDL_AST_NODE_TYPE_ATTR>()) {
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

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_VERSION>) {
        const auto optionVersion = node.ctx().options() ? node.ctx().options()->getVersion() : nullptr;
        auto& ctx                = node.ctx();
        auto arg0                = ctx.getNodeRef(argFrist);
        auto arg1                = arg0 ? ctx.getNodeRef(arg0->sibling) : ctx.emptyNodeRef();
        auto arg2                = arg1 ? ctx.getNodeRef(arg1->sibling) : ctx.emptyNodeRef();
        if (optionVersion) {
            arg0.setReplacedByCompiler();
            arg1.setReplacedByCompiler();
            arg2.setReplacedByCompiler();
            auto fail = false;
            if (optionVersion->major < 0 || optionVersion->major > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, optionVersion->major);
                fail = true;
            }
            if (optionVersion->minor < 0 || optionVersion->minor > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, optionVersion->minor);
                fail = true;
            }
            if (optionVersion->micro < 0 || optionVersion->micro > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, optionVersion->micro);
                fail = true;
            }
            if (!fail) {
                auto nodeMajor = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_LITERAL_INT);
                auto nodeMinor = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_LITERAL_INT);
                auto nodeMicro = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_LITERAL_INT);

                ctx.result()->getNode(nodeMajor)->valueInt = optionVersion->major;
                ctx.result()->getNode(nodeMinor)->valueInt = optionVersion->minor;
                ctx.result()->getNode(nodeMicro)->valueInt = optionVersion->micro;
                ctx.result()->getNode(nodeMajor)->sibling  = nodeMinor;
                ctx.result()->getNode(nodeMinor)->sibling  = nodeMicro;
                ctx.result()->getNode(nodeMicro)->sibling  = argFrist;
                arg0.setReplacedByCompiler();

                node->child = nodeMajor;
            }
        } else if (argCount == 3 && arg0.is<IDL_AST_NODE_TYPE_LITERAL_INT>() &&
                   arg1.is<IDL_AST_NODE_TYPE_LITERAL_INT>() && arg2.is<IDL_AST_NODE_TYPE_LITERAL_INT>()) {
            const auto major = arg0->valueInt;
            const auto minor = arg1->valueInt;
            const auto micro = arg2->valueInt;
            auto fail        = false;
            if (major < 0 || major > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, major);
                fail = true;
            }
            if (minor < 0 || minor > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, minor);
                fail = true;
            }
            if (micro < 0 || micro > 255) {
                ctx.log<IDL_STATUS_E3004>(node->location, micro);
                fail = true;
            }
            if (!fail) {
                node->child = argFrist;
            }
        } else if (argCount == 1 && arg0.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
            static const std::regex semverRegex(R"((0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*))");
            std::smatch matches;
            // Compatibility with old version format: "1.2.3" instead of "version(1, 2, 3)"
            auto strView = ctx.result()->getStr(arg0->valueStr);
            auto str     = std::string(strView.data(), strView.length());
            if (std::regex_match(str, matches, semverRegex)) {
                const auto major = std::stoll(matches[1].str());
                const auto minor = std::stoll(matches[2].str());
                const auto micro = std::stoll(matches[3].str());
                if (major <= 255 && minor <= 255 && micro <= 255) {
                    auto nodeMajor = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_LITERAL_INT);
                    auto nodeMinor = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_LITERAL_INT);
                    auto nodeMicro = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_LITERAL_INT);

                    ctx.result()->getNode(nodeMajor)->valueInt = major;
                    ctx.result()->getNode(nodeMinor)->valueInt = minor;
                    ctx.result()->getNode(nodeMicro)->valueInt = micro;
                    ctx.result()->getNode(nodeMajor)->sibling  = nodeMinor;
                    ctx.result()->getNode(nodeMinor)->sibling  = nodeMicro;
                    ctx.result()->getNode(nodeMicro)->sibling  = argFrist;
                    arg0.setReplacedByCompiler();

                    node->child = nodeMajor;
                    return;
                }
            }
            node->child = argFrist;
        } else {
            ctx.log<IDL_STATUS_E3003>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>) {
        if (argCount > 0) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_VALUE>) {
        if (argCount > 0) {
            auto arg0   = node.ctx().getNodeRef(argFrist);
            node->child = argFrist;
            auto view   = node | std::views::all;
            auto it     = std::ranges::find_if(view, [](const auto& val) {
                return val.is<IDL_AST_NODE_TYPE_LITERAL>();
            });
            ASTNodeType type;
            if (it == view.end()) {
                if (!arg0.is<IDL_AST_NODE_TYPE_DECL_REF>()) {
                    node.ctx().log<IDL_STATUS_E3025>(node->location);
                    node->child = HandleNone;
                    return;
                }
                type = IDL_AST_NODE_TYPE_DECL_REF;
            } else {
                type = it->type();
            }
            for (auto child : node) {
                if (child.is<IDL_AST_NODE_TYPE_LITERAL>()) {
                    if (child->type != type) {
                        node.ctx().log<IDL_STATUS_E3026>(node->location);
                        node->child = HandleNone;
                        break;
                    }
                } else if (!child.is<IDL_AST_NODE_TYPE_DECL_REF>()) {
                    node.ctx().log<IDL_STATUS_E3025>(node->location);
                    node->child = HandleNone;
                    break;
                }
            }
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_TYPE>) {
        auto arg0 = node.ctx().getNodeRef(argFrist);
        if (argCount == 1 && arg0.is<IDL_AST_NODE_TYPE_DECL_REF>()) {
            node->child = argFrist;
        } else {
            node.ctx().log<IDL_STATUS_E3027>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_CNAME>) {
        auto& ctx = node.ctx();
        auto arg0 = ctx.getNodeRef(argFrist);
        if (argCount == 1 && arg0.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
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

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>) {
        auto& ctx = node.ctx();
        if (argCount > 0) {
            node->child = argFrist;

            auto isAllIntegers = std::all_of(node.begin(), node.end(), [this](auto arg) {
                return arg.is<IDL_AST_NODE_TYPE_LITERAL_INT>();
            });
            if (!isAllIntegers) {
                auto arg0 = ctx.getNodeRef(argFrist);
                if (argCount == 1 && arg0.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                    static std::regex pattern(R"(\^?\d+(-\^?\d+)*)");
                    auto strView = arg0.valueStr();
                    auto str     = std::string(strView.data(), strView.length());
                    // Compatibility with old tokenizer format: "1-^2-3" instead of "tokenizer(1, -2, 3)"
                    if (std::regex_match(str, pattern)) {
                        std::stringstream ss(str);
                        std::string token;
                        ASTNode* lastNum = nullptr;
                        while (std::getline(ss, token, '-')) {
                            auto num = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_LITERAL_INT);
                            if (token[0] == '^') {
                                ctx.result()->getNode(num)->valueInt = -std::stoi(token.substr(1));
                            } else {
                                ctx.result()->getNode(num)->valueInt = std::stoi(token);
                            }
                            if (!lastNum) {
                                node->child = num;
                            } else {
                                lastNum->sibling = num;
                            }
                            lastNum = ctx.result()->getNode(num);
                        }
                        lastNum->sibling = argFrist;
                        arg0.setReplacedByCompiler();
                    } else {
                        ctx.log<IDL_STATUS_E3031>(node->location, str);
                        node->child = HandleNone;
                    }
                } else {
                    ctx.log<IDL_STATUS_E3032>(node->location);
                    node->child = HandleNone;
                }
            }
        } else {
            ctx.log<IDL_STATUS_E3014>(
                node->location, node.accept<AttrName>().str, " (integers: 2, -2, 4 or string \"2-^3-4\")");
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) {
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

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        auto& ctx = node.ctx();
        if (ctx.result()->getApi() != HandleNone) {
            ctx.log<IDL_STATUS_E3010>(node->location, node.name());
        } else {
            ctx.initBuiltins(node);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        if (checkApi(node.ctx())) {
            if (!node.ctx().findImport(node)) {
                auto parent  = findRoot(node.ctx());
                node->parent = parent.handle();
                parent.addChild(node);
            } else {
                node.setReplacedByCompiler();
            }
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        if (checkApi(node.ctx())) {
            auto parent  = findRoot(node.ctx());
            node->parent = parent.handle();
            parent.addChild(node);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        if (auto parent = findParent<IDL_AST_NODE_TYPE_ENUM>(node.ctx())) {
            node->parent = parent.handle();
            parent.addChild(node);
        } else {
            node.ctx().log<IDL_STATUS_E3023>(node->location, node.name());
            node->parent = node.ctx().result()->getApi();
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_STRUCT>) {
        if (checkApi(node.ctx())) {
            auto parent  = findRoot(node.ctx());
            node->parent = parent.handle();
            parent.addChild(node);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FIELD>) {
        if (auto parent = findParent<IDL_AST_NODE_TYPE_STRUCT>(node.ctx())) {
            node->parent = parent.handle();
            parent.addChild(node);
        } else {
            node.ctx().log<IDL_STATUS_E3019>(node->location, node.name());
            node->parent = node.ctx().result()->getApi();
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"no hierarchy rule is defined for this node type.");
    }

    static bool checkApi(Context& ctx) {
        if (ctx.result()->getApi() == HandleNone) {
            ctx.log<IDL_STATUS_E3011>(ASTLocation());
            return false;
        }
        return true;
    }

    template <ASTNodeType... Types>
    ASTNodeRef findParent(Context& ctx) noexcept {
        auto curr = ctx.getNodeRef(lastNode);
        while (curr.is<IDL_AST_NODE_TYPE_DECL>()) {
            if (curr.is<Types...>()) {
                return curr;
            }
            curr = curr.parent();
        }
        return ctx.emptyNodeRef();
    }

    ASTNodeRef findRoot(Context& ctx) noexcept {
        return findParent<IDL_AST_NODE_TYPE_API, IDL_AST_NODE_TYPE_IMPORT>(ctx);
    }

    ASTNodeHandle lastNode;
};

struct IntegerCastRules {
    explicit IntegerCastRules(ASTNodeRef& value) noexcept : value(value) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_INT_8>) {
        cast<int8_t>(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_UINT_8>) {
        cast<uint8_t>(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_INT_16>) {
        cast<int16_t>(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_UINT_16>) {
        cast<uint16_t>(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_INT_32>) {
        cast<int32_t>(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_UINT_32>) {
        cast<uint32_t>(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_INT_64>) {
        cast<int64_t>(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_UINT_64>) {
        cast<uint64_t>(node);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"only integer types");
    }

    template <typename T>
    void cast(ASTNodeRef& node) {
        std::string valueStr;
        if constexpr (std::numeric_limits<T>::is_signed) {
            const auto valueInt = value->valueInt;
            success = valueInt >= std::numeric_limits<T>::min() && valueInt <= std::numeric_limits<T>::max();
            if (!success) {
                valueStr = std::to_string(valueInt);
            }
        } else {
            const auto valueInt = uint64_t(value->valueInt);
            success = valueInt >= std::numeric_limits<T>::min() && valueInt <= std::numeric_limits<T>::max();
            if (!success) {
                valueStr = std::to_string(valueInt);
            }
        }
        if (!success) {
            node.ctx().log<IDL_STATUS_W2004>(
                value->location, node.name(), valueStr, std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        }
    }

    ASTNodeRef& value;
    bool success{ true };
};

struct BuildRules {
    struct State {
        bool prevE3041;
        std::vector<ASTNodeRef> enums;

        void clearNodes() noexcept {
            for (auto enumNode : enums) {
                auto& ctx   = enumNode.ctx();
                auto consts = enumNode.getChilds() | std::views::filter([](const auto& child) {
                    return child.is<IDL_AST_NODE_TYPE_CONST>();
                });
                for (auto constNode : consts) {
                    auto prevChild = ctx.emptyNodeRef();
                    for (auto child : constNode) {
                        if (child.is<IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF>()) {
                            break;
                        }
                        prevChild = child;
                    }
                    if (prevChild && prevChild->sibling != HandleNone) {
                        auto declPrevSiblingRef     = ctx.getNodeRef(prevChild->sibling);
                        prevChild->sibling          = declPrevSiblingRef->sibling;
                        declPrevSiblingRef->parent  = HandleNone;
                        declPrevSiblingRef->sibling = HandleNone;
                        declPrevSiblingRef->child   = HandleNone;
                    }
                }
            }
        }
    };

    BuildRules(State& state) noexcept : state(state) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        if (node.ctx().options() && node.ctx().options()->getOutputFiles() != IDL_OUTPUT_FILES_DEFAULT) {
            const auto format = node.ctx().options()->getOutputFiles();
            if (format == IDL_OUTPUT_FILES_SINGLE && !node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>()) {
                auto attrSinglehandle = node.ctx().result()->allocNode(node->location, IDL_AST_NODE_TYPE_ATTR_SINGLE);
                auto attrSingle       = node.ctx().getNodeRef(attrSinglehandle);
                attrSingle->parent    = node.handle();
                node.addChild(attrSingle);
            } else if (format == IDL_OUTPUT_FILES_MULTI && node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>()) {
                node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>().setReplacedByCompiler();
            }
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        auto& ctx = node.ctx();
        if (auto type = node.declType()) {
            if (!type.is<IDL_AST_NODE_TYPE_INTEGER_TYPE>()) {
                ctx.log<IDL_STATUS_E3044>(node->location, node.fullname());
            }
        } else {
            auto attrTypeHandle = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_ATTR_TYPE);
            auto attrType       = ctx.getNodeRef(attrTypeHandle);
            attrType->parent    = node.handle();
            attrType->child     = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_DECL_REF);

            auto declRef                 = ctx.getNodeRef(attrType->child);
            declRef->parent              = attrTypeHandle;
            declRef->valueDeclRef.symbol = ctx.result()->intern("Int32");

            node.addChild(attrType);
        }

        if (!node.findChild<IDL_AST_NODE_TYPE_CONST>()) {
            ctx.log<IDL_STATUS_E3045>(node->location, node.fullname());
            node.setBuildError();
        }

        auto enumConsts = node | std::views::filter([](const auto& child) {
            return child.is<IDL_AST_NODE_TYPE_CONST>();
        });

        ASTNodeRef prevEnumConst = ctx.emptyNodeRef();
        for (auto enumConst : enumConsts) {
            if (prevEnumConst) {
                auto prevSiblingHandle =
                    ctx.result()->allocNode(enumConst->location, IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF);
                auto prevSibling    = ctx.getNodeRef(prevSiblingHandle);
                prevSibling->parent = enumConst.handle();

                prevSibling->valueDeclRef.symbol = prevEnumConst->valueStr;
                prevSibling->valueDeclRef.handle = prevEnumConst.handle();
                prevSibling.setEvaulated();

                enumConst.addChild(prevSibling);
            }
            prevEnumConst = enumConst;
        }

        state.enums.push_back(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        evaulateEnumConst(node);
    }

    std::optional<uint64_t> evaulateEnumConst(ASTNodeRef& enumConst) {
        if (enumConst.evaulated()) {
            if (!enumConst.buildError()) {
                state.prevE3041 = false;
            }
            auto attrValue = enumConst.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>();
            assert(attrValue);
            return enumConst.buildError() ? std::nullopt : std::make_optional(attrValue->valueInt);
        }

        auto& ctx = enumConst.ctx();

        ASTNodeRef prevEnumConst = ctx.emptyNodeRef();
        if (auto prevSiblingRef = enumConst.findChild<IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF>()) {
            prevEnumConst = prevSiblingRef.resolveRef();
        }
        if (!prevEnumConst) {
            state.prevE3041 = false;
        }

        auto type = enumConst.parent().declType();

        if (auto attrValue = enumConst.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>()) {
            attrValue->valueInt = 0;
            for (auto value : attrValue) {
                if (value.is<IDL_AST_NODE_TYPE_LITERAL_INT>()) {
                    attrValue->valueInt |= value->valueInt;
                    continue;
                }

                if (!value.is<IDL_AST_NODE_TYPE_DECL_REF>()) {
                    ctx.log<IDL_STATUS_E3040>(value->location, enumConst.fullname());
                    enumConst.setBuildError();
                    break;
                }

                auto decl = value.resolveRef();
                if (!decl) {
                    enumConst.setBuildError();
                    break;
                }

                if (!decl.is<IDL_AST_NODE_TYPE_CONST>()) {
                    ctx.log<IDL_STATUS_E3038>(enumConst->location);
                    enumConst.setBuildError();
                    break;
                }

                if (decl == enumConst) {
                    ctx.log<IDL_STATUS_E3039>(enumConst->location, decl.fullname());
                    enumConst.setBuildError();
                    break;
                }

                if (!decl.evaulated()) {
                    ctx.log<IDL_STATUS_W2003>(enumConst->location, enumConst.fullname(), decl.fullname());
                    enumConst.setForwardDecl();

                    const auto [hasCyclic, deps] = findCyclicRelationshipConsts(enumConst, decl);
                    if (hasCyclic) {
                        ctx.log<IDL_STATUS_E3042>(enumConst->location, deps);
                        enumConst.setBuildError();
                        break;
                    }

                    if (!evaulateEnumConst(decl)) {
                        enumConst.setBuildError();
                    }
                }

                if (decl.buildError()) {
                    if (!state.prevE3041) {
                        state.prevE3041 = true;
                        ctx.log<IDL_STATUS_E3041>(enumConst->location, enumConst.fullname());
                    }
                    enumConst.setBuildError();
                    break;
                }

                const auto evaulatedValue = decl.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>()->valueInt;
                if (decl.parent() != enumConst.parent()) {
                    auto argValue      = ASTNodeRef::byType<IDL_AST_NODE_TYPE_LITERAL_INT>(ctx);
                    argValue->valueInt = evaulatedValue;
                    argValue->location = value->location;
                }

                attrValue->valueInt |= evaulatedValue;
            }
        } else {
            auto attrValueHandle = ctx.result()->allocNode(enumConst->location, IDL_AST_NODE_TYPE_ATTR_VALUE);
            attrValue            = ctx.getNodeRef(attrValueHandle);
            attrValue->parent    = enumConst.handle();
            attrValue->child     = ctx.result()->allocNode(enumConst->location, IDL_AST_NODE_TYPE_LITERAL_INT);
            attrValue->valueInt  = 0;

            ctx.result()->getNode(attrValue->child)->parent = attrValueHandle;
            if (prevEnumConst) {
                assert(prevEnumConst.evaulated());
                if (prevEnumConst.buildError()) {
                    if (!state.prevE3041) {
                        state.prevE3041 = true;
                        ctx.log<IDL_STATUS_E3041>(enumConst->location, enumConst.fullname());
                    }
                    enumConst.setBuildError();
                } else {
                    auto prevEvaulatedValue = prevEnumConst.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>()->valueInt;
                    attrValue->valueInt     = prevEvaulatedValue + 1;
                }
            } else {
                attrValue->valueInt = 0;
            }
            enumConst.addChild(attrValue);
        }

        auto attrValue = enumConst.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>();
        assert(attrValue);

        const auto warnAsErrors = ctx.options() ? ctx.options()->getWarningsAsErrors() : false;
        if (!type.is<IDL_AST_NODE_TYPE_INTEGER_TYPE>()) {
            attrValue->valueInt = 0;
            enumConst.setBuildError();
        } else if (!type.accept<IntegerCastRules>(attrValue).success && warnAsErrors) {
            enumConst.setBuildError();
        }

        if (!enumConst.buildError()) {
            state.prevE3041 = false;
        }

        enumConst.setEvaulated();
        return enumConst.buildError() ? std::nullopt : std::make_optional(attrValue->valueInt);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_DECL_REF>) {
        auto _ = node.resolveRef();
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
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

            buffer.clear();
            if (top.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>()) {
                auto declRefs =
                    top.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>() | std::views::filter([this](const auto& value) {
                    return value.is<IDL_AST_NODE_TYPE_DECL_REF>();
                });
                for (auto ref : declRefs) {
                    if (auto decl = ref.resolveRef(); decl.is<IDL_AST_NODE_TYPE_CONST>()) {
                        if (!cameFrom.contains(decl.handle().handle)) {
                            cameFrom[decl.handle().handle] = top;
                        } else if (decl == target) {
                            cameFrom[decl.handle().handle] = top;
                        }
                        buffer.push_back(decl);
                    }
                }
            } else if (auto prevSiblingRef = top.findChild<IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF>()) {
                auto decl = prevSiblingRef.resolveRef();
                if (!cameFrom.contains(decl.handle().handle)) {
                    cameFrom[decl.handle().handle] = top;
                } else if (decl == target) {
                    cameFrom[decl.handle().handle] = top;
                }
                buffer.push_back(decl);
            }
            for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
                consts.push(*it);
            }
        }
        return { false, "" };
    }

    State& state;
};

} // namespace idl

#endif
