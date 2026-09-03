#include "case_converter.hpp"
#include "fixed_stack.hpp"
#include "idl_resources.hpp"
#include "rules.hpp"
#include "visitors.hpp"
#include "writer.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace idl::gen::c {

namespace {

using OverrideDoc = std::vector<std::pair<std::string_view, std::vector<std::string>>>;

struct State;

struct Include {
    State& state;
    Output output;
    ASTNodeRef node;
    ASTNodeRef import;
    ASTNodeRef prevNode;
    inja::json data;
};

struct Stats {
    bool hasEnums{};
    bool hasEnumFlags{};
    bool addTypedEnumMacros{};
    ASTNodeRef voidType{};
    std::unordered_set<ASTNodeRef> hasImports;

    void build() {
        if (!hasEnums) {
            addTypedEnumMacros = false;
        }
    }
};

struct State {
    Writer& writer;
    Stats stats;
    CName::Cache cnameCache;
    inja::json data;
    inja::Environment env;
    std::vector<Include> includes;
    std::unordered_map<std::string, inja::Template> templates;

    std::ostream& out() noexcept {
        return includes.back().output.stream();
    }

    void mergeImport() {
        auto& import = includes.back().data;

        data["macros"]["include_guard"] = import["include_guard"];
        data["filename"]                = import["filename"];
        data["is_main"]                 = import["is_main"];
        data["has_includes"]            = import["has_includes"];
    }

    void popImport() {
        data["include_guard_close"] = true;
        env.render_to(out(), templates["c_include_guard.txt"], data);
        includes.pop_back();
        data["include_guard_close"] = false;
        if (!includes.empty()) {
            mergeImport();
        }
    }

    void flushImports() {
        while (!includes.empty()) {
            popImport();
        }
    }
};

struct DoxygenVisitor {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>) {
        str = "brief"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>) {
        str = "details"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_RETURN>) {
        str = "return"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>) {
        str = "author"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>) {
        str = "copyright"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ARG>) {
        str = "param[in]"sv;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    std::string_view str;
};

struct DoxygenGroupVisitor {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_API>) {
        str = "files"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        str = "files"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        str = "enums"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_STRUCT>) {
        str = "structs"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        str = "functions"sv;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    std::string_view str;
};

struct ASTStatsVisitor {
    explicit ASTStatsVisitor(Stats& stats) noexcept : stats(stats) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPED_ENUMS>()) {
            stats.addTypedEnumMacros = true;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        stats.hasImports.insert(node.parent());
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        stats.hasEnums = true;
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>()) {
            stats.hasEnumFlags = true;
        }
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPED_ENUMS>()) {
            stats.addTypedEnumMacros = true;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_VOID>) {
        stats.voidType = node;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    Stats& stats;
};

struct ASTVisitor {
    ASTVisitor(State& state) noexcept : state(state) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        addConfig(node);
        addCallbacks(node);
        addMacros(node);
        renderPlatformHeader(node);
        renderVersionHeader(node);
        pushImport(node, ""sv, true);
        setPrevNode(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        pushImport(node);
        setPrevNode(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        tryPopImport(node);

        state.data["node"]     = node.handle().handle;
        state.data["is_flags"] = !!node.findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>();
        fillDoc(0, false, node, state.data);
        auto& docs = state.data["doxygen"]["docs"];
        docs.insert(docs.begin(),
                    inja::json{
                        { "name",     "typedef"                                                         },
                        { "literals", std::vector{ node.accept<CName>(std::ref(state.cnameCache)).str } }
        });

        auto& consts = state.data["consts"];
        for (auto child : node.getChilds<IDL_AST_NODE_TYPE_CONST>()) {
            consts.push_back(inja::json::object({
                { "node", child.handle().handle }
            }));
            fillDoc(1, true, child, consts.back());
        }

        auto useTypedEnums = !!node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPED_ENUMS>();
        if (!useTypedEnums) {
            useTypedEnums = !!state.writer.api().findChild<IDL_AST_NODE_TYPE_ATTR_TYPED_ENUMS>();
        }

        state.data["use_typed_enums"] = useTypedEnums;

        state.env.render_to(state.out(), findTemplate("c_enum.txt"), state.data);

        state.data.erase("use_typed_enums");
        state.data.erase("consts");

        setPrevNode(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_STRUCT>) {
        tryPopImport(node);

        state.data["node"] = node.handle().handle;
        fillDoc(0, false, node, state.data);

        auto& fields = state.data["fields"];
        for (auto child : node.getChilds<IDL_AST_NODE_TYPE_FIELD>()) {
            fields.push_back(inja::json::object({
                { "node", child.handle().handle }
            }));
            fillDoc(1, true, child, fields.back());
        }

        state.env.render_to(state.out(), findTemplate("c_struct.txt"), state.data);

        state.data.erase("fields");

        setPrevNode(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        tryPopImport(node);

        state.data["node"] = node.handle().handle;
        fillDoc(0, false, node, state.data);

        auto& args = state.data["args"];
        for (auto child : node.getChilds<IDL_AST_NODE_TYPE_ARG>()) {
            args.push_back(inja::json::object({
                { "node", child.handle().handle }
            }));
        }

        state.env.render_to(state.out(), findTemplate("c_func.txt"), state.data);

        state.data.erase("args");

        setPrevNode(node);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    void fillDoc(size_t level, bool isInline, ASTNodeRef node, inja::json& data, const OverrideDoc& overrideDoc = {}) {
        const auto indents = state.data["config"]["indents"].get<size_t>();

        data.erase("doxygen");
        auto& doxygen        = data["doxygen"];
        doxygen["indents"]   = std::string(level * indents, ' ');
        doxygen["is_inline"] = isInline;
        doxygen["config"]    = state.data["config"];

        auto& docs = doxygen["docs"];
        docs       = {};

        std::map<int, ASTNodeRef> nodes;

        if (node.is<IDL_AST_NODE_TYPE_IMPORT>()) {
            for (auto doc : state.writer.api().getChilds<IDL_AST_NODE_TYPE_ATTR_DOC>()) {
                if (!doc.is<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>()) {
                    nodes[doc.accept<PriorityDocAttr>().prior] = doc;
                }
            }
        }

        for (auto doc : node.getChilds<IDL_AST_NODE_TYPE_ATTR_DOC>()) {
            if (!doc.is<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>()) {
                nodes[doc.accept<PriorityDocAttr>().prior] = doc;
            }
        }

        if (node.is<IDL_AST_NODE_TYPE_API, IDL_AST_NODE_TYPE_IMPORT>()) {
            const auto& filename = state.includes.back().output.filename();
            docs.push_back({
                { "name",     "file" },
                { "literals", {}     }
            });
            docs.back()["literals"].push_back(filename.string());
        }

        int copyrightPos = -1;
        std::unordered_map<std::string_view, size_t> map;
        for (auto [_, doc] : nodes) {
            if (doc.is<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>()) {
                copyrightPos = (int) docs.size();
            }
            const auto name = doc.accept<DoxygenVisitor>().str;
            map[name]       = docs.size();
            docs.push_back({
                { "name",     name },
                { "literals", {}   }
            });

            auto& literals = docs.back()["literals"];
            for (auto literal : doc.getChilds()) {
                if (isInline && literal.is<IDL_AST_NODE_TYPE_LITERAL_STR>() && literal.valueStr()[0] == '\n') {
                    doxygen["is_inline"] = false;
                    isInline             = false;
                }
                literals.push_back(literal.handle().handle);
            }
        }

        if (state.data["config"]["add_doc_groups"].get<bool>()) {
            auto groupStr = node.accept<DoxygenGroupVisitor>().str;
            if (!groupStr.empty()) {
                auto groupIt = docs.end();
                if (copyrightPos >= 0) {
                    groupIt = docs.begin() + copyrightPos;
                }
                inja::json group = {
                    { "name",     "ingroup"    },
                    { "literals", { groupStr } }
                };
                docs.insert(groupIt, group);
            }
        }

        if (node.is<IDL_AST_NODE_TYPE_FUNC>()) {
            size_t offsetParams = 0;
            if (auto pos = map.find("details"); pos != map.end()) {
                offsetParams = pos->second + 1;
            } else if (auto pos = map.find("return"); pos != map.end()) {
                offsetParams = pos->second;
            }
            auto paramIt = docs.begin() + offsetParams;

            for (auto arg : node.getChilds<IDL_AST_NODE_TYPE_ARG>()) {
                inja::json param = {
                    { "name",     arg.accept<DoxygenVisitor>().str                            },
                    { "literals", { arg.accept<CName>(std::ref(state.cnameCache)).str, " "s } }
                };
                auto& literals = param["literals"];
                for (auto literal : arg.findChild<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>().getChilds()) {
                    if (isInline && literal.is<IDL_AST_NODE_TYPE_LITERAL_STR>() && literal.valueStr()[0] == '\n') {
                        doxygen["is_inline"] = false;
                        isInline             = false;
                    }
                    literals.push_back(literal.handle().handle);
                }
                paramIt = docs.insert(paramIt, param);
            }
        }

        for (const auto& [name, literals] : overrideDoc) {
            inja::json doc = {
                { "name",     name     },
                { "literals", literals }
            };
            if (auto it = map.find(name); it != map.end()) {
                docs.at(it->second) = doc;
            } else {
                docs.push_back(doc);
            }
        }

        if (auto license = node.findChild<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>()) {
            if (state.data["is_main"].get<bool>()) {
                auto& literals = doxygen["license"];
                for (auto literal : license.getChilds()) {
                    literals.push_back(literal.handle().handle);
                }
            }
        }
    }

    inja::Template& findTemplate(const std::string& tmp) {
        if (auto it = state.templates.find(tmp); it != state.templates.end()) {
            return it->second;
        }
        std::string_view tmpName(tmp.c_str(), tmp.length());
        auto it = std::find_if(std::begin(resources::resource_table), std::end(resources::resource_table), [tmpName](const auto& item) {
            return item.first == tmpName;
        });
        assert(it != std::end(resources::resource_table));
        state.templates[tmp] = state.env.parse(it->second);
        return state.templates[tmp];
    }

    void addConfig(ASTNodeRef node) {
        struct Semver {
            int64_t major;
            int64_t minor;
            int64_t micro;
        };

        std::variant<std::string_view, Semver> version;
        auto attrVer = node.findChild<IDL_AST_NODE_TYPE_ATTR_VERSION>();
        assert(attrVer);
        auto literals = attrVer.getChilds<IDL_AST_NODE_TYPE_LITERAL>();
        std::vector<ASTNodeRef> args;
        args.assign(literals.begin(), literals.end());
        if (args.size() == 1 && args[0].is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
            version = args[0].valueStr();
        } else if (args.size() == 3 && args[0].is<IDL_AST_NODE_TYPE_LITERAL_INT>() && args[1].is<IDL_AST_NODE_TYPE_LITERAL_INT>() &&
                   args[2].is<IDL_AST_NODE_TYPE_LITERAL_INT>()) {
            version = Semver{ args[0]->valueInt, args[1]->valueInt, args[2]->valueInt };
        }

        constexpr auto filters = ASTNodeRef::SkipDocs | ASTNodeRef::SkipAttrs | ASTNodeRef::SkipLiterals;
        state.writer.api().acceptRecursive<ASTStatsVisitor>(filters, std::ref(state.stats));
        state.stats.build();

        auto& cname       = state.cnameCache;
        auto attrBoolType = node.findChild<IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE>();
        assert(attrBoolType);
        auto boolType = node.ctx().getNodeRef(attrBoolType->child).resolveRef(true);
        if (boolType.is<IDL_AST_NODE_TYPE_INT_8>()) {
            cname.boolType = BoolType::Int8;
        } else if (boolType.is<IDL_AST_NODE_TYPE_INT_32>()) {
            cname.boolType = BoolType::Int32;
        } else if (boolType.is<IDL_AST_NODE_TYPE_BOOL>()) {
            cname.boolType = BoolType::StdBool;
        }

        std::unordered_map<std::string_view, CFormatRule> cformat;
        for (const auto& format : CFormatRules::formats) {
            cformat[format.name] = format;
        }
        if (auto cformatAttr = node.findChild<IDL_AST_NODE_TYPE_ATTR_CFORMAT>()) {
            auto view = cformatAttr.getChilds() | std::views::transform([](ASTNodeRef arg) {
                assert(arg.is<IDL_AST_NODE_TYPE_LITERAL_STR>());
                auto str = arg.valueStr();
                auto pos = str.find_last_of(':');
                assert(pos != std::string_view::npos);
                return std::make_pair(str.substr(0, pos), str.substr(pos + 1));
            });
            for (const auto& [key, value] : view) {
                auto& format = cformat[key];
                switch (format.type) {
                    case CFormatRule::Value:
                        try {
                            std::string str{ value.data(), value.length() };
                            format.value = std::stoi(str);
                        } catch (const std::exception&) {
                        }
                        break;
                    case CFormatRule::Bool:
                        format.value = value[0] == 't';
                        break;
                    case CFormatRule::Choice: {
                        auto it = std::find(format.choices.begin(), format.choices.end(), value);
                        if (it != format.choices.end()) {
                            format.value = int(std::distance(format.choices.begin(), it));
                        }
                        break;
                    }
                }
            }
        }

        auto single          = !!node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>();
        auto addDoc          = false;
        auto addDocGroups    = false;
        auto addMemberGroups = false;
        cname.stdTypes       = !!node.findChild<IDL_AST_NODE_TYPE_ATTR_STD_TYPES>();
        if (auto options = state.writer.options()) {
            auto cOptions   = options->getCOptions();
            addDoc          = cOptions.add_doc;
            addDocGroups    = cOptions.add_doc_groups;
            addMemberGroups = cOptions.add_member_groups;
        }

        auto& config                    = state.data["config"];
        config["single"]                = single;
        config["add_doc"]               = addDoc;
        config["add_doc_groups"]        = addDoc && addDocGroups;
        config["add_member_groups"]     = addDoc && addMemberGroups;
        config["std_types"]             = cname.stdTypes;
        config["has_enums"]             = state.stats.hasEnums;
        config["has_enum_flags"]        = state.stats.hasEnumFlags;
        config["add_typed_enum_macros"] = state.stats.addTypedEnumMacros;
        switch (cname.boolType) {
            case BoolType::Int32:
                config["bool_type"] = "int32";
                break;
            case BoolType::Int8:
                config["bool_type"] = "int8";
                break;
            case BoolType::StdBool:
                config["bool_type"] = "std_bool";
                break;
            default:
                assert(!"unreachable code");
                break;
        }
        for (const auto& [_, format] : cformat) {
            size_t start = 0;
            size_t pos;
            auto* curr = &config;
            while ((pos = format.name.substr(start).find_first_of('.')) != std::string_view::npos) {
                auto subname = format.name.substr(start, pos);
                curr         = &((*curr)[subname]);
                start += pos + 1;
            }
            curr = &((*curr)[format.name.substr(start)]);
            switch (format.type) {
                case CFormatRule::Value:
                    (*curr) = std::get<int>(format.value);
                    break;
                case CFormatRule::Bool:
                    (*curr) = std::get<bool>(format.value);
                    break;
                case CFormatRule::Choice:
                    (*curr) = format.choices[std::get<int>(format.value)];
                    break;
            }
        }
        config["semver"] = std::holds_alternative<Semver>(version);
        if (std::holds_alternative<Semver>(version)) {
            const auto& ver         = std::get<Semver>(version);
            config["version_major"] = ver.major;
            config["version_minor"] = ver.minor;
            config["version_micro"] = ver.micro;
        } else {
            config["version_string"] = std::get<std::string_view>(version);
        }

        std::vector<int> nums{};
        if (auto attr = node.findChild<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>()) {
            auto view = attr.getChilds() | std::views::transform([](const auto& arg) {
                return int(arg->valueInt);
            });
            nums.assign(view.begin(), view.end());
        }
        const auto str = node.name();

        state.data["api_name"]          = calcMacro(node, {}, false, false);
        state.data["api_name_readable"] = convert({ str.data(), str.length() }, Case::SpaceCase, nums.empty() ? nullptr : &nums);
        state.data["cvoid"]             = state.stats.voidType.accept<CName>(std::ref(state.cnameCache)).str;
    }

    void addCallbacks(ASTNodeRef node) {
        state.env.add_callback("cname", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            ASTNodeRef node(ctx, { handle });
            return node.accept<CName>(std::ref(state.cnameCache)).str;
        });

        state.env.add_callback("cnativetype", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            ASTNodeRef node(ctx, { handle });
            return node.accept<CNativeType>(state.cnameCache.boolType).str;
        });

        state.env.add_callback("ctype", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            ASTNodeRef node(ctx, { handle });
            return node.declType().accept<CName>(std::ref(state.cnameCache)).str;
        });

        state.env.add_callback("cvalue", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            auto node   = ASTNodeRef(ctx, { handle });
            return node.accept<CValue>(std::ref(state.cnameCache)).str;
        });

        state.env.add_callback("cliteral", 1, [this](inja::Arguments& args) {
            auto& ctx = state.writer.api().ctx();
            if (args.at(0)->type() == nlohmann::detail::value_t::string) {
                return args.at(0)->get<std::string>();
            } else {
                auto handle = args.at(0)->get<uint16_t>();
                auto node   = ASTNodeRef(ctx, { handle });
                return node.accept<CLiteral>(std::ref(state.cnameCache)).str;
            }
        });

        state.env.add_callback("render", 2, [this](inja::Arguments& args) {
            auto& data = *args.at(1);
            auto tmp   = findTemplate(args.at(0)->get<std::string>() + ".txt");
            return state.env.render(tmp, data);
        });

        state.env.add_callback("indents", 1, [this](inja::Arguments& args) {
            return std::string(args.at(0)->get<size_t>(), ' ');
        });

        state.env.add_callback("nl", 1, [this](inja::Arguments& args) {
            return std::string(args.at(0)->get<size_t>(), '\n');
        });

        state.env.add_callback("str", 1, [this](inja::Arguments& args) {
            const auto type = args.at(0)->type();
            const auto& arg = args.at(0);
            switch (type) {
                case nlohmann::detail::value_t::null:
                    return "NULL"s;
                case nlohmann::detail::value_t::string:
                    return escape(arg->get<std::string_view>());
                case nlohmann::detail::value_t::boolean:
                    return arg->get<bool>() ? "true"s : "false"s;
                case nlohmann::detail::value_t::number_integer:
                    return std::to_string(arg->get<int64_t>());
                case nlohmann::detail::value_t::number_unsigned:
                    return std::to_string(arg->get<uint64_t>());
                case nlohmann::detail::value_t::number_float:
                    return fmt::format("{:g}", arg->get<double>());
                default:
                    assert(!"unreachable code");
                    return ""s;
            }
        });
    }

    void addMacros(ASTNodeRef node) {
        auto prefix      = calcMacro(node, {}, false);
        auto prefixLower = calcMacro(node, {}, false, false);

        auto& macros                 = state.data["macros"];
        macros["macro_prefix"]       = prefix;
        macros["macro_prefix_lower"] = prefixLower;
        macros["import_api"]         = prefixLower + "_api"s;
        macros["static_build"]       = prefix + "_STATIC_BUILD"s;
        macros["platform_windows"]   = prefix + "_PLATFORM_WINDOWS"s;
        macros["platform_ios"]       = prefix + "_PLATFORM_IOS"s;
        macros["platform_macos"]     = prefix + "_PLATFORM_MAC_OS"s;
        macros["platform_android"]   = prefix + "_PLATFORM_ANDROID"s;
        macros["platform_linux"]     = prefix + "_PLATFORM_LINUX"s;
        macros["platform_web"]       = prefix + "_PLATFORM_WEB"s;
        macros["constexpr14"]        = prefix + "_CONSTEXPR_14"s;
        macros["version_major"]      = prefix + "_VERSION_MAJOR"s;
        macros["version_minor"]      = prefix + "_VERSION_MINOR"s;
        macros["version_micro"]      = prefix + "_VERSION_MICRO"s;
        macros["version_encode"]     = prefix + "_VERSION_ENCODE"s;
        macros["version_stringize_"] = prefix + "_VERSION_STRINGIZE_"s;
        macros["version_stringize"]  = prefix + "_VERSION_STRINGIZE"s;
        macros["version"]            = prefix + "_VERSION"s;
        macros["version_string"]     = prefix + "_VERSION_STRING"s;
        macros["begin_enum"]         = prefix + "_BEGIN_ENUM"s;
        macros["end_enum"]           = prefix + "_END_ENUM"s;
        macros["flags_enum"]         = prefix + "_FLAGS_ENUM"s;
    }

    void renderPlatformHeader(ASTNodeRef node) {
        std::vector brief   = { "Platform-specific definitions and utilities."s };
        std::vector details = { "This header provides cross-platform macros, type definitions, and utility"s,
                                "\n"s,
                                "macros for the "s,
                                state.data["api_name_readable"].get<std::string>(),
                                " library. It handles:"s,
                                "\n"s,
                                "  - Platform detection (Windows, macOS, iOS, Android, Linux, Web)"s,
                                "\n"s,
                                "  - Symbol visibility control (DLL import/export on Windows)"s,
                                "\n"s,
                                "  - C/C++ interoperability"s,
                                "\n"s,
                                "  - Type definitions for consistent data sizes across platforms"s,
                                "\n"s,
                                "  - Bit flag operations for enumerations (C++ only)."s,
                                "\n"s };

        auto& trivials = state.data["trivials"];
        for (auto child : node.getChilds<IDL_AST_NODE_TYPE_TRIVIAL_TYPE>()) {
            if (!child.is<IDL_AST_NODE_TYPE_VOID>()) {
                trivials.push_back(inja::json::object({
                    { "node", child.handle().handle }
                }));
                fillDoc(0, true, child, trivials.back());
            }
        }

        auto filename = pushImport(node,
                                   "Platform"sv,
                                   false,
                                   {
                                       { "brief",   std::move(brief)   },
                                       { "details", std::move(details) }
        });

        state.data["config"]["platform_header"] = filename;

        state.env.render_to(state.out(), findTemplate("c_platform.txt"), state.data);

        state.data.erase("trivials");
    }

    void renderVersionHeader(ASTNodeRef node) {
        const auto semver = state.data["config"]["semver"].get<bool>();
        std::vector<std::string> brief;
        std::vector<std::string> details;

        if (semver) {
            brief   = { "Library version information and utilities."s };
            details = { "This header provides version information for the "s,
                        state.data["api_name_readable"].get<std::string>(),
                        " library,"s,
                        "\n"s,
                        "including version number components and macros for version comparison"s,
                        "\n"s,
                        "and string generation. It supports:"s,
                        "\n"s,
                        "  - Major/Minor/Micro version components"s,
                        "\n"s,
                        "  - Integer version encoding"s,
                        "\n"s,
                        "  - String version generation"s,
                        "\n"s };
        } else {
            brief   = { "Library version information."s };
            details = { "This header provides version information for the "s, state.data["api_name_readable"].get<std::string>(), " library."s };
        }

        auto filename = pushImport(node,
                                   "Version"sv,
                                   false,
                                   {
                                       { "brief",   std::move(brief)   },
                                       { "details", std::move(details) }
        });

        state.data["config"]["version_header"] = filename;

        state.env.render_to(state.out(), findTemplate(semver ? "c_semver.txt"s : "c_strver.txt"s), state.data);
    }

    std::ostream& out() noexcept {
        return state.out();
    }

    void setPrevNode(ASTNodeRef& node) {
        state.includes.back().prevNode = node;
    }

    std::string pushImport(ASTNodeRef& node, std::string_view postfix = ""sv, bool main = false, const OverrideDoc& overrideDoc = {}) {
        const auto single = state.data["config"]["single"].get<bool>();
        if (single && !state.includes.empty()) {
            return {};
        }
        std::string name;
        std::string guard;
        if (!postfix.empty() && !single) {
            name  = filename(node, postfix);
            guard = calcMacro(node, postfix);
        } else {
            name  = filename(node);
            guard = calcMacro(node);
        }
        auto import = node.is<IDL_AST_NODE_TYPE_IMPORT>() ? node : node.ctx().emptyNodeRef();

        inja::json data;
        data["include_guard"] = guard;
        data["filename"]      = name;
        data["is_main"]       = main || single;
        data["has_includes"]  = state.stats.hasImports.contains(node);

        if (node.is<IDL_AST_NODE_TYPE_IMPORT>()) {
            auto it = std::find_if(state.includes.rbegin(), state.includes.rend(), [parent = node.parent()](auto& item) {
                return item.node == parent;
            });
            assert(it != state.includes.rend());
            if (!it->prevNode.is<IDL_AST_NODE_TYPE_IMPORT>()) {
                fmt::print(it->output.stream(), "\n");
            }
            fmt::print(it->output.stream(), "#include \"{}\"\n", name);
            it->prevNode = node;
        }

        state.includes.emplace_back(state, state.writer.createOutput(name), node, import, node.ctx().emptyNodeRef(), std::move(data));
        state.mergeImport();
        fillDoc(0, false, node, state.data, !postfix.empty() && !single ? overrideDoc : OverrideDoc{});

        state.env.render_to(state.out(), findTemplate("c_include_guard.txt"), state.data);

        return name;
    }

    void tryPopImport(ASTNodeRef& node) {
        const auto single = state.data["config"]["single"].get<bool>();
        if (single) {
            return;
        }
        auto currImport = state.includes.back().import;
        auto currParent = node.parent();
        while (currParent && !currParent.is<IDL_AST_NODE_TYPE_IMPORT>()) {
            currParent = currParent.parent();
        }
        if (currParent != currImport) {
            while (currParent != state.includes.back().import) {
                state.popImport();
            }
        }
    }

    std::string filename(ASTNodeRef& node, std::string_view postfix = {}) {
        return node.accept<CName>(std::ref(state.cnameCache), postfix).str;
    }

    std::string calcMacro(ASTNodeRef& node, std::string_view namePostfix = {}, bool addExt = true, bool upper = true) {
        node.accept<CName>(std::ref(state.cnameCache), namePostfix);
        CName::Cache cache = state.cnameCache;

        cache.conventions[IDL_AST_NODE_TYPE_API].caseConvention    = upper ? Case::ScreamingSnakeCase : Case::SnakeCase;
        cache.conventions[IDL_AST_NODE_TYPE_IMPORT].caseConvention = upper ? Case::ScreamingSnakeCase : Case::SnakeCase;
        if (!addExt) {
            cache.conventions[IDL_AST_NODE_TYPE_API].postfix    = {};
            cache.conventions[IDL_AST_NODE_TYPE_IMPORT].postfix = {};
        }

        auto prefix = node.accept<CName>(std::ref(cache), namePostfix).str;
        std::replace(prefix.begin(), prefix.end(), '-', '_');
        std::replace(prefix.begin(), prefix.end(), '.', '_');
        return prefix;
    }

    static std::string escape(std::string_view str) {
        std::ostringstream ss;
        ss << '"';
        for (char c : str) {
            switch (c) {
                case '\n':
                    ss << '\\' << 'n';
                    break;
                case '\t':
                    ss << '\\' << 't';
                    break;
                default:
                    if (c == '\\' || c == '"') {
                        ss << '\\';
                    }
                    ss << c;
                    break;
            }
        }
        ss << '"';
        return ss.str();
    }

    State& state;
};

} // namespace

void generate(Writer& writer) {
    constexpr auto filters = ASTNodeRef::SkipDocs | ASTNodeRef::SkipAttrs | ASTNodeRef::SkipLiterals | ASTNodeRef::SkipTrivials;
    State state{ writer };
    writer.api().acceptRecursive<ASTVisitor>(filters, std::ref(state));
    state.flushImports();
}

} // namespace idl::gen::c
