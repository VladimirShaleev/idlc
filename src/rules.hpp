#ifndef IDL_RULES_HPP
#define IDL_RULES_HPP

#include "visitors.hpp"

namespace idl {

struct AttrSubValidatorRules {
    AttrSubValidatorRules(ASTNodeType attrType, bool attached) noexcept : attrType(attrType), attached(attached) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        if (attrType == IDL_AST_NODE_TYPE_ATTR_DOC_RETURN) {
            if (auto declType = node.declType(); !declType || declType.is<IDL_AST_NODE_TYPE_VOID>()) {
                printWarn = false;
                if (declType) {
                    node.ctx().log<IDL_STATUS_N1005>(node->location);
                }
                if (attached) {
                    const auto token = node.accept<DeclToken>().str;
                    node.ctx().log<IDL_STATUS_W2008>(node->location, token, node.fullname());
                }
            }
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
    }

    ASTNodeType attrType;
    bool attached;
    bool printWarn{ true };
};

struct AttrValidatorRules {
    struct AttrInfo {
        std::string_view name;
        bool recommended;
    };

    AttrValidatorRules(Context& ctx) noexcept : ctx(ctx) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_VERSION>(),         add<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true),  add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),       add<IDL_AST_NODE_TYPE_ATTR_SINGLE>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_STD_TYPES>(),       add<IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>(true),  add<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>(true), add<IDL_AST_NODE_TYPE_ATTR_COUNT_ENUMS>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_MAX_ENUM>(),        add<IDL_AST_NODE_TYPE_ATTR_CCONV>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_CFORMAT>() };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(true), add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true) };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_FLAGS>(),         add<IDL_AST_NODE_TYPE_ATTR_HEX>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_COUNT_ENUMS>(),   add<IDL_AST_NODE_TYPE_ATTR_MAX_ENUM>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(true), add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),         add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TYPE>() };
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
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true), add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),      add<IDL_AST_NODE_TYPE_ATTR_TYPE>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_VALUE>(),          add<IDL_AST_NODE_TYPE_ATTR_REF>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_ARRAY>(),          add<IDL_AST_NODE_TYPE_ATTR_CONST>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_OPTIONAL>() };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        static std::map allowed = {
            add<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(true), add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true), add<IDL_AST_NODE_TYPE_ATTR_DOC_RETURN>(true),
            add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),         add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),      add<IDL_AST_NODE_TYPE_ATTR_TYPE>(),
            add<IDL_AST_NODE_TYPE_ATTR_REF>(),           add<IDL_AST_NODE_TYPE_ATTR_ARRAY>(),          add<IDL_AST_NODE_TYPE_ATTR_CONST>(),
            add<IDL_AST_NODE_TYPE_ATTR_OPTIONAL>()
        };
        validate(node, allowed);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ARG>) {
        static std::map allowed = { add<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(true), add<IDL_AST_NODE_TYPE_ATTR_CNAME>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>(),      add<IDL_AST_NODE_TYPE_ATTR_TYPE>(true),
                                    add<IDL_AST_NODE_TYPE_ATTR_VALUE>(),          add<IDL_AST_NODE_TYPE_ATTR_REF>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_ARRAY>(),          add<IDL_AST_NODE_TYPE_ATTR_CONST>(),
                                    add<IDL_AST_NODE_TYPE_ATTR_OPTIONAL>() };
        validate(node, allowed);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        if (node.is<IDL_AST_NODE_TYPE_DECL>()) {
            assert(!"Attributes are not allowed for the declaration");
        } else {
            assert(!"attempt to validate attributes for a non-declaration node");
        }
    }

    void validate(ASTNodeRef& node, const std::map<ASTNodeType, AttrInfo>& allowed) {
        auto fieldNames = allowed | std::views::values | std::views::transform([](const AttrInfo& info) {
            return info.name;
        });

        auto names = fmt::format("{}", fmt::join(fieldNames, ", "));
        std::set<ASTNodeType> uniqueAttrs;
        for (auto attr : node.getAttrs()) {
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
            auto containsAttr = uniqueAttrs.contains(type);
            auto printWarn    = node.accept<AttrSubValidatorRules>(type, containsAttr).printWarn;
            if (!containsAttr && printWarn) {
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
    std::string_view getName() {
        return ASTNodeRef::byType<Type>(ctx).template accept<AttrName>().str;
    }

    Context& ctx;
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
                                if (!str.empty() && str[0] == ' ') {
                                    minIndents = std::min(minIndents, str.length());
                                }
                            }
                            isNewLine = str.empty() || str[0] == '\n';
                        }
                    }
                    isNewLine    = true;
                    auto prevArg = node.ctx().emptyNodeRef();
                    for (auto arg : node) {
                        if (arg.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                            auto str = arg.valueStr();
                            if (isNewLine) {
                                if (!str.empty() && str[0] == ' ') {
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
                            }
                            isNewLine = str.empty() || str[0] == '\n';
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

struct CFormatRule {
    enum Type {
        Value,
        Bool,
        Choice
    };

    std::string_view name;
    Type type;
    std::array<std::string_view, 5> choices;
    std::variant<int, bool> value;
};

struct CFormatRules {
    static constexpr std::array formats = {
        CFormatRule{ "indents",                 CFormatRule::Value,  {},                             4     },
        CFormatRule{ "space.after.comma",       CFormatRule::Bool,   {},                             true  },
        CFormatRule{ "arg.alignment",           CFormatRule::Choice, { "none", "bracket" },          1     },
        CFormatRule{ "arg.wrapping",            CFormatRule::Choice, { "none", "each_on_wew_line" }, 1     },
        CFormatRule{ "break.after.api",         CFormatRule::Bool,   {},                             false },
        CFormatRule{ "break.after.decl",        CFormatRule::Bool,   {},                             true  },
        CFormatRule{ "break.after.return.type", CFormatRule::Bool,   {},                             true  },
    };

    static std::regex regex() {
        std::ostringstream ss;
        for (const auto& [name, type, choices, value] : formats) {
            ss << '(';
            for (char c : name) {
                if (c == '.') {
                    ss << '\\';
                }
                ss << c;
            }
            ss << ':';
            switch (type) {
                case CFormatRule::Value:
                    ss << "\\d+";
                    break;
                case CFormatRule::Bool:
                    ss << "(true|false)";
                    break;
                case CFormatRule::Choice: {
                    auto hasPrev = false;
                    ss << '(';
                    for (auto choice : choices) {
                        if (choice.empty()) {
                            break;
                        }
                        if (hasPrev) {
                            ss << '|';
                        }
                        ss << choice;
                        hasPrev = true;
                    }
                    ss << ')';
                    break;
                }
            }
            ss << ")|";
        }
        auto format = ss.str();
        format.pop_back();
        return std::regex(format);
    }

    static std::string correctFormat() {
        std::ostringstream ss;
        for (const auto& [name, type, choices, value] : formats) {
            ss << name << ":<";
            switch (type) {
                case CFormatRule::Value:
                    ss << "value(" << std::get<int>(value);
                    break;
                case CFormatRule::Bool:
                    ss << "bool(" << (std::get<bool>(value) ? "true" : "false");
                    break;
                case CFormatRule::Choice: {
                    auto hasPrev = false;
                    for (auto choice : choices) {
                        if (choice.empty()) {
                            break;
                        }
                        if (hasPrev) {
                            ss << '|';
                        }
                        ss << choice;
                        hasPrev = true;
                    }
                    ss << '(' << choices[std::get<int>(value)];
                    break;
                }
            }
            ss << ")>, ";
        }
        ss << "sample: " << formats[0].name << ':' << std::get<int>(formats[0].value);
        return ss.str();
    }
};

struct AttrArgRules {
    AttrArgRules(ASTNodeHandle argFirst, ASTNodeHandle argLast, size_t argCount) noexcept : argFirst(argFirst), argLast(argLast), argCount(argCount) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_VERSION>) {
        const auto optionVersion = node.ctx().options() ? node.ctx().options()->getVersion() : nullptr;
        auto& ctx                = node.ctx();
        auto arg0                = ctx.getNodeRef(argFirst);
        auto arg1                = arg0 ? ctx.getNodeRef(arg0->sibling) : ctx.emptyNodeRef();
        auto arg2                = arg1 ? ctx.getNodeRef(arg1->sibling) : ctx.emptyNodeRef();
        if (argCount == 3 && arg0.is<IDL_AST_NODE_TYPE_LITERAL_INT>() && arg1.is<IDL_AST_NODE_TYPE_LITERAL_INT>() &&
            arg2.is<IDL_AST_NODE_TYPE_LITERAL_INT>()) {
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
                node->child = argFirst;
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
                    node->child = argFirst;
                    arg0.setReplacedByCompiler();
                    ctx.addLiteral(node, int64_t(major));
                    ctx.addLiteral(node, int64_t(minor));
                    ctx.addLiteral(node, int64_t(micro));
                    return;
                }
            }
            node->child = argFirst;
        } else {
            ctx.log<IDL_STATUS_E3003>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE>) {
        if (argCount == 1) {
            auto arg0 = node.ctx().getNodeRef(argFirst);
            if (arg0.is<IDL_AST_NODE_TYPE_DECL_REF>()) {
                auto type = node.result()->getStr(arg0->valueDeclRef.symbol);
                if (type == "Int8" || type == "Int32" || type == "Bool") {
                    node->child = argFirst;
                    return;
                }
            }
        }
        node.ctx().log<IDL_STATUS_E3050>(node->location, node.accept<AttrName>().str, " (Int8, Int32 or Bool)");
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>) {
        if (argCount > 0) {
            node->child = argFirst;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>) {
        if (argCount > 0) {
            node->child = argFirst;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>) {
        if (argCount > 0) {
            node->child = argFirst;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_RETURN>) {
        if (argCount > 0) {
            node->child = argFirst;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>) {
        if (argCount > 0) {
            node->child = argFirst;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>) {
        if (argCount > 0) {
            node->child = argFirst;
        } else {
            node.ctx().log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, "");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_VALUE>) {
        if (argCount > 0) {
            auto arg0   = node.ctx().getNodeRef(argFirst);
            node->child = argFirst;
            auto view   = node | std::views::all;
            auto it     = std::ranges::find_if(view, [](const auto& val) {
                return val.template is<IDL_AST_NODE_TYPE_LITERAL>();
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
        auto arg0 = node.ctx().getNodeRef(argFirst);
        if (argCount == 1 && arg0.is<IDL_AST_NODE_TYPE_DECL_REF>()) {
            node->child = argFirst;
        } else {
            node.ctx().log<IDL_STATUS_E3027>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_CNAME>) {
        auto& ctx = node.ctx();
        auto arg0 = ctx.getNodeRef(argFirst);
        if (argCount == 1 && arg0.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
            auto name = arg0.valueStr();
            if (std::any_of(name.begin(), name.end(), [](auto ch) {
                return !(std::isalpha(ch) || std::isdigit(ch) || ch == '_');
            })) {
                ctx.log<IDL_STATUS_E3029>(node->location, name);
            } else {
                node->child = argFirst;
            }
        } else {
            ctx.log<IDL_STATUS_E3028>(node->location);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_CCONV>) {
        using namespace std::string_view_literals;
        auto& ctx          = node.ctx();
        node->child        = argFirst;
        const auto tokens  = cconvTokens(ctx);
        const auto types   = cconvTypes();
        const auto cases   = cconvCase();
        const auto sumbols = "[a-zA-Z0-9_\\-^\\.@]*"sv;
        const std::regex format(fmt::format("({0}|{1}):({2}):(full|short):(skip|add):{3}:{3}(:{3})?", tokens, types, cases, sumbols));
        int index = -1;
        std::string_view invalidArg{};

        auto isValidFormats = argCount > 0 && std::all_of(node.begin(), node.end(), [this, &format, &index, &invalidArg](auto arg) {
            ++index;
            if (arg.template is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                auto argView = arg.valueStr();
                invalidArg   = argView;
                return std::regex_match(argView.begin(), argView.end(), format);
            }
            return false;
        });
        constexpr std::string_view correctFormat = "<token>:<snake|camel|pascal|...>:<full|short>:<skip|add>:<prefix>:<postfix>[:<special_postfix>], "
                                                   "sample: const:screamingsnake:full:skip:::_BIT";
        if (argCount == 0) {
            node->child = HandleNone;
            ctx.log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, std::format(" (format: {})", correctFormat));
        } else if (!isValidFormats) {
            node->child = HandleNone;
            ctx.log<IDL_STATUS_E3058>(node->location, index, invalidArg, correctFormat);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_CFORMAT>) {
        using namespace std::string_view_literals;
        auto& ctx   = node.ctx();
        node->child = argFirst;
        int index   = -1;
        std::string_view invalidArg{};
        const auto format = CFormatRules::regex();

        auto isValidFormats      = argCount > 0 && std::all_of(node.begin(), node.end(), [this, &format, &index, &invalidArg](auto arg) {
            ++index;
            if (arg.template is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                auto argView = arg.valueStr();
                invalidArg   = argView;
                return std::regex_match(argView.begin(), argView.end(), format);
            }
            return false;
        });
        const auto correctFormat = CFormatRules::correctFormat();
        if (argCount == 0) {
            node->child = HandleNone;
            ctx.log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, std::format(" (format: {})", correctFormat));
        } else if (!isValidFormats) {
            node->child = HandleNone;
            ctx.log<IDL_STATUS_E3059>(node->location, index, invalidArg, correctFormat);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>) {
        auto& ctx = node.ctx();
        if (argCount > 0) {
            node->child = argFirst;

            auto isAllIntegers = std::all_of(node.begin(), node.end(), [this](auto arg) {
                return arg.template is<IDL_AST_NODE_TYPE_LITERAL_INT>();
            });
            if (!isAllIntegers) {
                auto arg0 = ctx.getNodeRef(argFirst);
                if (argCount == 1 && arg0.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                    static std::regex pattern(R"(\^?\d+(-\^?\d+)*)");
                    auto strView = arg0.valueStr();
                    auto str     = std::string(strView.data(), strView.length());
                    // Compatibility with old tokenizer format: "1-^2-3" instead of "tokenizer(1, -2, 3)"
                    if (std::regex_match(str, pattern)) {
                        std::stringstream ss(str);
                        std::string token;
                        node->child = argFirst;
                        arg0.setReplacedByCompiler();
                        while (std::getline(ss, token, '-')) {
                            if (token[0] == '^') {
                                ctx.addLiteral(node, int64_t(-std::stoi(token.substr(1))));
                            } else {
                                ctx.addLiteral(node, int64_t(std::stoi(token)));
                            }
                        }
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
            ctx.log<IDL_STATUS_E3014>(node->location, node.accept<AttrName>().str, " (integers: 2, -2, 4 or string \"2-^3-4\")");
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_ARRAY>) {
        auto& ctx = node.ctx();
        auto arg0 = ctx.getNodeRef(argFirst);
        if (argCount == 1 && (arg0.is<IDL_AST_NODE_TYPE_LITERAL_INT, IDL_AST_NODE_TYPE_LITERAL_STR>())) {
            if (arg0.is<IDL_AST_NODE_TYPE_LITERAL_INT>()) {
                if (arg0->valueInt > 0) {
                    node->child = argFirst;
                } else {
                    ctx.log<IDL_STATUS_E3051>(node->location);
                }
            } else {
                arg0.setReplacedByCompiler();
                node->child = argFirst;
                ctx.addLiteral(node, DeclRef(arg0->valueStr));
            }
        } else {
            ctx.log<IDL_STATUS_E3050>(
                node->location, node.accept<AttrName>().str, " (fixed-size integer or reference to an integer field/arg specifying the size)");
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) {
        if (argCount > 0) {
            const auto name = node.accept<AttrName>().str;
            node.ctx().log<IDL_STATUS_E3008>(node->location, name);
        }
    }

    template <size_t... I>
    static std::string cconvTokensSeq(Context& ctx, std::index_sequence<I...>) {
        std::ostringstream ss;
        ((ss << ASTNodeRef::byType<idl_ast_node_type_t(I + IDL_AST_NODE_TYPE_ENUM)>(ctx).accept<DeclToken>().str << '|'), ...);
        ss << "file";
        return ss.str();
    }

    static std::string cconvTokens(Context& ctx) {
        constexpr size_t count = IDL_AST_NODE_TYPE_DECL_REF - IDL_AST_NODE_TYPE_ENUM;
        return cconvTokensSeq(ctx, std::make_index_sequence<count>{});
    }

    template <ASTNodeType Type>
    static std::string cconvType() {
        auto name = magic_enum::enum_name(Type).substr(18);
        std::string str{ name.data(), name.length() };
        if (auto pos = str.find_last_of('_'); pos != std::string::npos) {
            str.erase(pos, 1);
        }
        return lower(str);
    }

    template <size_t B, size_t... I>
    static void cconvTypesSeq(std::ostringstream& ss, std::index_sequence<I...>) {
        ((ss << cconvType<ASTNodeType(B + I)>() << '|'), ...);
    }

    static std::string cconvTypes() {
        constexpr size_t otherCount   = IDL_AST_NODE_TYPE_INTEGER_TYPE - IDL_AST_NODE_TYPE_VOID;
        constexpr size_t integerCount = IDL_AST_NODE_TYPE_FLOAT_TYPE - IDL_AST_NODE_TYPE_INT_8;
        constexpr size_t floatCount   = IDL_AST_NODE_TYPE_FLOAT_64 - IDL_AST_NODE_TYPE_FLOAT_TYPE;

        std::ostringstream ss;
        cconvTypesSeq<IDL_AST_NODE_TYPE_VOID>(ss, std::make_index_sequence<otherCount>{});
        cconvTypesSeq<IDL_AST_NODE_TYPE_INT_8>(ss, std::make_index_sequence<integerCount>{});
        cconvTypesSeq<IDL_AST_NODE_TYPE_FLOAT_32>(ss, std::make_index_sequence<floatCount>{});
        auto result = ss.str();
        result.pop_back();
        return result;
    }

    static std::string cconvCase() {
        std::ostringstream ss;
        for (const auto& name : magic_enum::enum_names<Case>()) {
            auto view = name.substr(0, name.length() - 4) | std::views::transform([](auto c) {
                return char(std::tolower(c));
            });
            for (auto c : view) {
                ss << c;
            }
            ss << '|';
        }
        auto result = ss.str();
        result.pop_back();
        return result;
    }

    ASTNodeHandle argFirst;
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

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        if (checkApi(node.ctx())) {
            auto parent  = findRoot(node.ctx());
            node->parent = parent.handle();
            parent.addChild(node);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ARG>) {
        if (auto parent = findParent<IDL_AST_NODE_TYPE_FUNC>(node.ctx())) {
            node->parent = parent.handle();
            parent.addChild(node);
        } else {
            node.ctx().log<IDL_STATUS_E3057>(node->location, node.name());
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
            success             = valueInt >= std::numeric_limits<T>::min() && valueInt <= std::numeric_limits<T>::max();
            if (!success) {
                valueStr = std::to_string(valueInt);
            }
        } else {
            const auto valueInt = uint64_t(value->valueInt);
            success             = valueInt >= std::numeric_limits<T>::min() && valueInt <= std::numeric_limits<T>::max();
            if (!success) {
                valueStr = std::to_string(valueInt);
            }
        }
        if (!success) {
            node.ctx().log<IDL_STATUS_W2004>(value->location, node.name(), valueStr, std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        }
    }

    ASTNodeRef& value;
    bool success{ true };
};

struct FloatCastRules {
    explicit FloatCastRules(ASTNodeRef& value) noexcept : value(value) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FLOAT_32>) {
        cast<float>(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FLOAT_64>) {
        cast<double>(node);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"only float types");
    }

    template <typename T>
    void cast(ASTNodeRef& node) {
        std::string valueStr;
        const auto valueFloat = value->valueFloat;
        success               = valueFloat >= std::numeric_limits<T>::lowest() && valueFloat <= std::numeric_limits<T>::max();
        if (!success) {
            valueStr = fmt::format("{:g}", valueFloat);
            node.ctx().log<IDL_STATUS_W2004>(value->location, node.name(), valueStr, std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max());
        }
    }

    ASTNodeRef& value;
    bool success{ true };
};

struct DefaultValueRules {
    explicit DefaultValueRules(ASTNodeRef& type, std::string_view declaration) noexcept : type(type), declaration(declaration) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_STR>) {
        if (!type.is<IDL_AST_NODE_TYPE_STR>()) {
            node.ctx().log<IDL_STATUS_E3035>(node->location, "string", type.fullname());
            success = false;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_INT>) {
        if (type.is<IDL_AST_NODE_TYPE_FLOAT_TYPE>()) {
            auto& ctx = node.ctx();
            node.setReplacedByCompiler();
            auto nodeFloatHandle  = ctx.result()->allocNode(node->location, IDL_AST_NODE_TYPE_LITERAL_FLOAT);
            auto nodeFloat        = ctx.getNodeRef(nodeFloatHandle);
            nodeFloat->valueFloat = (double) node->valueInt;
            nodeFloat->parent     = node->parent;
            nodeFloat->sibling    = node->sibling;
            node->sibling         = nodeFloatHandle;
            skipNext              = true;
            ctx.log<IDL_STATUS_W2007>(node->location);
            if (!type.accept<FloatCastRules>(nodeFloat).success) {
                success = false;
            }
        } else if (!type.is<IDL_AST_NODE_TYPE_INTEGER_TYPE>()) {
            node.ctx().log<IDL_STATUS_E3035>(node->location, "integer", type.fullname());
            success = false;
        } else if (!type.accept<IntegerCastRules>(node).success) {
            success = false;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_BOOL>) {
        if (!type.is<IDL_AST_NODE_TYPE_BOOL>()) {
            node.ctx().log<IDL_STATUS_E3035>(node->location, "boolean", type.fullname());
            success = false;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_FLOAT>) {
        if (!type.is<IDL_AST_NODE_TYPE_FLOAT_TYPE>()) {
            node.ctx().log<IDL_STATUS_E3035>(node->location, "float-point", type.fullname());
            success = false;
        } else if (!type.accept<FloatCastRules>(node).success) {
            success = false;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_DECL_REF>) {
        if (auto ref = node.resolveRef(); !ref.is<IDL_AST_NODE_TYPE_CONST>()) {
            node.ctx().log<IDL_STATUS_E3048>(node->location);
            success = false;
        } else if (!type.is<IDL_AST_NODE_TYPE_ENUM>()) {
            node.ctx().log<IDL_STATUS_E3035>(node->location, "enum const", type.fullname());
            success = false;
        } else if (type != ref.parent()) {
            node.ctx().log<IDL_STATUS_E3049>(node->location, ref.name(), ref.parent().fullname(), declaration, type.fullname());
            success = false;
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"only literal types");
    }

    ASTNodeRef& type;
    std::string_view declaration;
    bool success{ true };
    bool skipNext{};
};

struct BuildRules {
    struct State {
        bool prevE3041;
        std::vector<ASTNodeRef> enums;

        void clearNodes() noexcept {
            for (auto enumNode : enums) {
                auto& ctx   = enumNode.ctx();
                auto consts = enumNode.getChilds() | std::views::filter([](const auto& child) {
                    return child.template is<IDL_AST_NODE_TYPE_CONST>();
                });
                for (auto constNode : consts) {
                    auto prevChild = ctx.emptyNodeRef();
                    for (auto child : constNode) {
                        if (child.template is<IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF>()) {
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
        auto& ctx = node.ctx();
        if (const auto options = ctx.options()) {
            if (options->getOutputFiles() != IDL_OUTPUT_FILES_DEFAULT) {
                const auto format = options->getOutputFiles();
                if (format == IDL_OUTPUT_FILES_SINGLE && !node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>()) {
                    ctx.addNode<IDL_AST_NODE_TYPE_ATTR_SINGLE>(node);
                } else if (format == IDL_OUTPUT_FILES_MULTI && node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>()) {
                    node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>().setReplacedByCompiler();
                }
            }
            if (!node.findChild<IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE>()) {
                ctx.addNode<IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE>(node, DeclRef("Int32"));
            }
            if (const auto ver = options->getVersion()) {
                if (auto attrVer = node.findChild<IDL_AST_NODE_TYPE_ATTR_VERSION>()) {
                    attrVer.setReplacedByCompiler();
                }
                if (ver->str) {
                    std::string_view verStr = ver->str;
                    ctx.addNode<IDL_AST_NODE_TYPE_ATTR_VERSION>(node, verStr);
                } else {
                    auto fail = false;
                    if (ver->major < 0 || ver->major > 255) {
                        ctx.log<IDL_STATUS_E3004>(node->location, ver->major);
                        fail = true;
                    }
                    if (ver->major < 0 || ver->major > 255) {
                        ctx.log<IDL_STATUS_E3004>(node->location, ver->minor);
                        fail = true;
                    }
                    if (ver->minor < 0 || ver->micro > 255) {
                        ctx.log<IDL_STATUS_E3004>(node->location, ver->micro);
                        fail = true;
                    }
                    if (!fail) {
                        ctx.addNode<IDL_AST_NODE_TYPE_ATTR_VERSION>(node, int64_t(ver->major), int64_t(ver->minor), int64_t(ver->micro));
                    }
                }
            } else if (!node.findChild<IDL_AST_NODE_TYPE_ATTR_VERSION>()) {
                ctx.addNode<IDL_AST_NODE_TYPE_ATTR_VERSION>(node, int64_t(0), int64_t(0), int64_t(0));
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
            node.addDeclType<IDL_AST_NODE_TYPE_INT_32>();
        }

        if (!node.findChild<IDL_AST_NODE_TYPE_CONST>()) {
            ctx.log<IDL_STATUS_E3045>(node->location, node.fullname());
            node.setBuildError();
        }

        auto enumConsts = node | std::views::filter([](const auto& child) {
            return child.template is<IDL_AST_NODE_TYPE_CONST>();
        });

        ASTNodeRef prevEnumConst = ctx.emptyNodeRef();
        for (auto enumConst : enumConsts) {
            if (prevEnumConst) {
                auto prevSibling = ctx.addNode<IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF>(enumConst);

                prevSibling->valueDeclRef.symbol = prevEnumConst->valueStr;
                prevSibling->valueDeclRef.handle = prevEnumConst.handle();
                prevSibling.setEvaulated();
            }
            prevEnumConst = enumConst;
        }
        node.setEvaulated();

        state.enums.push_back(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        evaulateEnumConst(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_STRUCT>) {
        if (!node.findChild<IDL_AST_NODE_TYPE_FIELD>()) {
            node.ctx().log<IDL_STATUS_E3024>(node->location, node.fullname());
            node.setBuildError();
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FIELD>) {
        auto& ctx = node.ctx();
        if (!node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPE>()) {
            node.addDeclType<IDL_AST_NODE_TYPE_INT_32>();
        }
        auto type = node.declType();
        if (!type) {
            node.setBuildError();
            return;
        }
        const auto warnAsErrors = ctx.options() ? ctx.options()->getWarningsAsErrors() : false;

        auto attrType = node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPE>();
        if (type.is<IDL_AST_NODE_TYPE_STRUCT>() && !type.evaulated()) {
            auto hasRef = !!node.findChild<IDL_AST_NODE_TYPE_ATTR_REF>();
            if (node.parent() == type) {
                if (!hasRef) {
                    ctx.log<IDL_STATUS_E3033>(attrType->location, node.fullname(), type.fullname());
                    node.setBuildError();
                }
            } else {
                node.parent().setForwardDecl();
                if (!hasRef) {
                    ctx.log<IDL_STATUS_E3030>(attrType->location, node.fullname(), type.fullname());
                    node.setBuildError();
                }
            }
        } else if (!type.is<IDL_AST_NODE_TYPE_STRUCT>() && !type.evaulated()) {
            node.parent().setForwardDecl();
            ctx.log<IDL_STATUS_W2006>(attrType->location, node.fullname(), type.accept<DeclToken>().str, type.fullname());
            if (warnAsErrors) {
                node.setBuildError();
            }
        } else if (type.is<IDL_AST_NODE_TYPE_VOID>()) {
            ctx.log<IDL_STATUS_E3034>(attrType->location, node.accept<DeclToken>().str, node.fullname());
            node.setBuildError();
        }
        assingValue(node, type);
        node.setEvaulated();
        if (node->sibling == HandleNone) {
            node.parent().setEvaulated();
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        auto& ctx = node.ctx();
        if (!node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPE>()) {
            node.addDeclType<IDL_AST_NODE_TYPE_VOID>();
        }
        auto type = node.declType();
        if (!type) {
            node.setBuildError();
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ARG>) {
        auto& ctx = node.ctx();
        auto type = node.declType();
        if (!type || type.is<IDL_AST_NODE_TYPE_VOID>()) {
            auto attrType = node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPE>();
            ctx.log<IDL_STATUS_E3060>(attrType ? attrType->location : node->location, node.fullname());
            node.setBuildError();
        }
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

        auto attrValue = enumConst.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>();
        if (attrValue) {
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
            if (prevEnumConst) {
                assert(prevEnumConst.evaulated());
                if (prevEnumConst.buildError()) {
                    if (!state.prevE3041) {
                        state.prevE3041 = true;
                        ctx.log<IDL_STATUS_E3041>(enumConst->location, enumConst.fullname());
                    }
                    attrValue           = ctx.addNode<IDL_AST_NODE_TYPE_ATTR_VALUE>(enumConst, int64_t(0));
                    attrValue->valueInt = 0;
                    enumConst.setBuildError();
                } else {
                    auto prevEvaulatedValue = prevEnumConst.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>()->valueInt;
                    attrValue               = ctx.addNode<IDL_AST_NODE_TYPE_ATTR_VALUE>(enumConst, int64_t(prevEvaulatedValue + 1));
                    attrValue->valueInt     = int64_t(prevEvaulatedValue + 1);
                }
            } else {
                attrValue           = ctx.addNode<IDL_AST_NODE_TYPE_ATTR_VALUE>(enumConst, int64_t(0));
                attrValue->valueInt = 0;
            }
        }
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

        if (!enumConst.nextSibling<IDL_AST_NODE_TYPE_CONST>()) {
            using namespace std::string_view_literals;

            auto addCountEnums = !!enumConst.parent().findChild<IDL_AST_NODE_TYPE_ATTR_COUNT_ENUMS>();
            if (!addCountEnums) {
                addCountEnums = !!ctx.getNodeRef(ctx.result()->getApi()).findChild<IDL_AST_NODE_TYPE_ATTR_COUNT_ENUMS>();
            }

            auto addMaxEnum = !!enumConst.parent().findChild<IDL_AST_NODE_TYPE_ATTR_MAX_ENUM>();
            if (!addMaxEnum) {
                addMaxEnum = !!ctx.getNodeRef(ctx.result()->getApi()).findChild<IDL_AST_NODE_TYPE_ATTR_MAX_ENUM>();
            }

            if (addCountEnums) {
                auto countValue = 0ll;
                auto curr       = enumConst;
                while (curr) {
                    ++countValue;
                    if (auto prev = curr.findChild<IDL_AST_NODE_TYPE_DECL_PREV_SIBLING_REF>()) {
                        curr = prev.resolveRef();
                    } else {
                        break;
                    }
                };

                auto countEnums       = ctx.addNode<IDL_AST_NODE_TYPE_CONST>(enumConst.parent());
                countEnums->name.name = ctx.result()->intern("CountEnums");
                countEnums.setBuiltin();

                ctx.addNode<IDL_AST_NODE_TYPE_ATTR_VALUE>(countEnums, int64_t(countValue))->valueInt = countValue;
                ctx.addNode<IDL_AST_NODE_TYPE_ATTR_COUNT_ENUMS>(countEnums).setBuiltin();
                ctx.addNode<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(
                    countEnums, "Count"sv, " "sv, "of"sv, " "sv, "constants"sv, " "sv, "in"sv, " "sv, "the"sv, " "sv, "enumeration."sv);

                countEnums.setEvaulated();
                ctx.addSymbol(countEnums.handle());
            }

            if (addMaxEnum) {
                auto maxValue      = int64_t(0x7FFFFFFF);
                auto maxEnum       = ctx.addNode<IDL_AST_NODE_TYPE_CONST>(enumConst.parent());
                maxEnum->name.name = ctx.result()->intern("MaxEnum");
                maxEnum.setBuiltin();

                ctx.addNode<IDL_AST_NODE_TYPE_ATTR_VALUE>(maxEnum, maxValue)->valueInt = maxValue;
                ctx.addNode<IDL_AST_NODE_TYPE_ATTR_MAX_ENUM>(maxEnum).setBuiltin();
                ctx.addNode<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(
                    maxEnum, "Max"sv, " "sv, "value"sv, " "sv, "of"sv, " "sv, "enum"sv, " "sv, "(not"sv, " "sv, "used)."sv);

                maxEnum.setEvaulated();
                ctx.addSymbol(maxEnum.handle());
            }
        }

        enumConst.setEvaulated();
        return enumConst.buildError() ? std::nullopt : std::make_optional(attrValue->valueInt);
    }

    void assingValue(ASTNodeRef& node, ASTNodeRef& type) {
        auto valueAttr   = node.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>();
        auto declaration = node.accept<DeclToken>().str;
        auto& ctx        = node.ctx();
        auto attrArray   = node.findChild<IDL_AST_NODE_TYPE_ATTR_ARRAY>();
        if (auto declRef = attrArray.findChild<IDL_AST_NODE_TYPE_DECL_REF>()) {
            if (auto decl = declRef.resolveRef()) {
                if (decl == node) {
                    ctx.log<IDL_STATUS_E3054>(node->location, declaration, node.fullname());
                    node.setBuildError();
                } else if (decl.parent() != node.parent()) {
                    ctx.log<IDL_STATUS_E3053>(node->location, declaration, node.fullname());
                    node.setBuildError();
                } else if (!decl.declType().is<IDL_AST_NODE_TYPE_INTEGER_TYPE>() || decl.findChild<IDL_AST_NODE_TYPE_ATTR_ARRAY>()) {
                    std::string_view typeArray = decl.findChild<IDL_AST_NODE_TYPE_ATTR_ARRAY>() ? "array of " : "";
                    ctx.log<IDL_STATUS_E3052>(node->location, declaration, decl.fullname(), typeArray, decl.declType().fullname());
                    node.setBuildError();
                }
            } else {
                node.setBuildError();
            }
        }
        int countArgs = 0;
        auto skipNext = false;
        for (auto value : valueAttr) {
            if (!skipNext) {
                auto result = value.accept<DefaultValueRules>(type, declaration);
                skipNext    = result.skipNext;
                ++countArgs;
                if (!result.success) {
                    node.setBuildError();
                }
            }
        }
        if (countArgs > 1) {
            auto isEnumFlags = type.is<IDL_AST_NODE_TYPE_ENUM>() && type.findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>();
            if (!isEnumFlags && !attrArray) {
                if (type.is<IDL_AST_NODE_TYPE_ENUM>()) {
                    ctx.log<IDL_STATUS_E3056>(node->location, declaration, node.fullname());
                } else {
                    ctx.log<IDL_STATUS_E3055>(node->location, declaration, node.fullname());
                }
                node.setBuildError();
            }
            if (auto fixedSize = attrArray.findChild<IDL_AST_NODE_TYPE_LITERAL_INT>(); fixedSize && fixedSize->valueInt < countArgs) {
                ctx.log<IDL_STATUS_E3006>(node->location, countArgs, declaration, node.fullname(), fixedSize->valueInt);
                node.setBuildError();
            }
        }
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
                auto attrValue = top.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>();
                auto declRefs  = attrValue | std::views::filter([this](const ASTNodeRef& value) {
                    return value.template is<IDL_AST_NODE_TYPE_DECL_REF>();
                });
                for (auto ref : declRefs) {
                    if (auto decl = ref.resolveRef(); decl.template is<IDL_AST_NODE_TYPE_CONST>()) {
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
