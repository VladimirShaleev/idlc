#include "case_converter.hpp"
#include "fixed_stack.hpp"
#include "idl_resources.hpp"
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
    ASTNodeRef import;
    inja::json data;
};

struct State {
    Writer& writer;
    bool stdTypes;
    idl_bool_type_t boolType;
    inja::json data;
    inja::Environment env;
    std::stack<Include> includes;
    std::unordered_map<std::string, inja::Template> templates;

    std::ostream& out() noexcept {
        return includes.top().output.stream();
    }

    void mergeImport() {
        auto& import = includes.top().data;

        data["macros"]["include_guard"] = import["include_guard"];
        data["is_main"]                 = import["is_main"];
    }

    void flushImports() {
        data["include_guard_close"] = true;
        while (!includes.empty()) {
            env.render_to(out(), templates["c_include_guard.txt"], data);
            includes.pop();
            if (!includes.empty()) {
                mergeImport();
            }
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

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>) {
        str = "author"sv;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>) {
        str = "copyright"sv;
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

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    std::string_view str;
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
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        pushImport(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        tryPopImport(node);

        state.data["node"] = node.handle().handle;
        fillDoc(0, false, node, state.data);

        auto& consts = state.data["consts"];
        for (auto child : node.getChilds<IDL_AST_NODE_TYPE_CONST>()) {
            consts.push_back(inja::json::object({
                { "node", child.handle().handle }
            }));
            fillDoc(1, true, child, consts.back());
        }

        state.env.render_to(state.out(), findTemplate("c_enum.txt"), state.data);

        state.data.erase("consts");
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
            const auto& filename = state.includes.top().output.filename();
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
            auto groupIt = docs.end();
            if (copyrightPos >= 0) {
                groupIt = docs.begin() + copyrightPos;
            }
            inja::json group = {
                { "name",     "ingroup"                                  },
                { "literals", { node.accept<DoxygenGroupVisitor>().str } }
            };
            docs.insert(groupIt, group);
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
        if (auto ver = node.findChild<IDL_AST_NODE_TYPE_ATTR_VERSION>()) {
            auto literals = ver.getChilds<IDL_AST_NODE_TYPE_LITERAL>();
            std::vector<ASTNodeRef> args;
            args.assign(literals.begin(), literals.end());
            if (args.size() == 1 && args[0].is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                version = args[0].valueStr();
            } else if (args.size() == 3 && args[0].is<IDL_AST_NODE_TYPE_LITERAL_INT>() && args[1].is<IDL_AST_NODE_TYPE_LITERAL_INT>() &&
                       args[2].is<IDL_AST_NODE_TYPE_LITERAL_INT>()) {
                version = Semver{ args[0]->valueInt, args[1]->valueInt, args[2]->valueInt };
            }
        } else {
            version = Semver{};
        }

        if (auto attrBoolType = node.findChild<IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE>()) {
            auto boolType = node.ctx().getNodeRef(attrBoolType->child).resolveRef(true);
            if (boolType.is<IDL_AST_NODE_TYPE_INT_8>()) {
                state.boolType = IDL_BOOL_TYPE_INT_8;
            } else if (boolType.is<IDL_AST_NODE_TYPE_INT_32>()) {
                state.boolType = IDL_BOOL_TYPE_INT_32;
            } else if (boolType.is<IDL_AST_NODE_TYPE_BOOL>()) {
                state.boolType = IDL_BOOL_TYPE_STD_BOOL;
            }
        } else {
            state.boolType = IDL_BOOL_TYPE_INT_32;
        }

        auto indents         = 4;
        auto single          = !!node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>();
        auto addDoc          = false;
        auto addDocGroups    = false;
        auto addMemberGroups = false;
        state.stdTypes       = !!node.findChild<IDL_AST_NODE_TYPE_ATTR_STD_TYPES>();
        if (auto options = state.writer.options()) {
            indents         = options->getIndents();
            auto cOptions   = options->getCOptions();
            addDoc          = cOptions.add_doc;
            addDocGroups    = cOptions.add_doc_groups;
            addMemberGroups = cOptions.add_member_groups;
        }
        auto& config                = state.data["config"];
        config["single"]            = single;
        config["indents"]           = indents;
        config["add_doc"]           = addDoc;
        config["add_doc_groups"]    = addDoc && addDocGroups;
        config["add_member_groups"] = addDoc && addMemberGroups;
        config["std_types"]         = state.stdTypes;
        switch (state.boolType) {
            case IDL_BOOL_TYPE_INT_32:
                config["bool_type"] = "int32";
                break;
            case IDL_BOOL_TYPE_INT_8:
                config["bool_type"] = "int8";
                break;
            case IDL_BOOL_TYPE_STD_BOOL:
                config["bool_type"] = "std_bool";
                break;
            default:
                assert(!"unreachable code");
                break;
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
    }

    void addCallbacks(ASTNodeRef node) {
        state.env.add_callback("cname", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            ASTNodeRef node(ctx, { handle });
            return node.accept<CName>(state.stdTypes, state.boolType).str;
        });

        state.env.add_callback("cnativetype", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            ASTNodeRef node(ctx, { handle });
            return node.accept<CNativeType>(state.boolType).str;
        });

        state.env.add_callback("ctype", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            ASTNodeRef node(ctx, { handle });
            return node.declType().accept<CName>(state.stdTypes, state.boolType).str;
        });

        state.env.add_callback("cvalue", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            auto node   = ASTNodeRef(ctx, { handle });
            return node.accept<CValue>(state.stdTypes, state.boolType).str;
        });

        state.env.add_callback("cliteral", 1, [this](inja::Arguments& args) {
            auto& ctx = state.writer.api().ctx();
            if (args.at(0)->type() == nlohmann::detail::value_t::string) {
                return args.at(0)->get<std::string>();
            } else {
                auto handle = args.at(0)->get<uint16_t>();
                auto node   = ASTNodeRef(ctx, { handle });
                return node.accept<CLiteral>(state.stdTypes, state.boolType).str;
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
        auto& macros                 = state.data["macros"];
        macros["import_api"]         = fullname(node, false, '_', "api"sv);
        macros["static_build"]       = fullname(node, true, '_', "STATIC"sv, "BUILD"sv);
        macros["platform_windows"]   = fullname(node, true, '_', "PLATFORM"sv, "WINDOWS"sv);
        macros["platform_ios"]       = fullname(node, true, '_', "PLATFORM"sv, "IOS"sv);
        macros["platform_macos"]     = fullname(node, true, '_', "PLATFORM"sv, "MAC"sv, "OS"sv);
        macros["platform_android"]   = fullname(node, true, '_', "PLATFORM"sv, "ANDROID"sv);
        macros["platform_linux"]     = fullname(node, true, '_', "PLATFORM"sv, "LINUX"sv);
        macros["platform_web"]       = fullname(node, true, '_', "PLATFORM"sv, "WEB"sv);
        macros["constexpr14"]        = fullname(node, true, '_', "CONSTEXPR"sv, "14"sv);
        macros["version_major"]      = fullname(node, true, '_', "VERSION"sv, "MAJOR"sv);
        macros["version_minor"]      = fullname(node, true, '_', "VERSION"sv, "MINOR"sv);
        macros["version_micro"]      = fullname(node, true, '_', "VERSION"sv, "MICRO"sv);
        macros["version_encode"]     = fullname(node, true, '_', "VERSION"sv, "ENCODE"sv);
        macros["version_stringize_"] = fullname(node, true, '_', "VERSION"sv, "STRINGIZE_"sv);
        macros["version_stringize"]  = fullname(node, true, '_', "VERSION"sv, "STRINGIZE"sv);
        macros["version"]            = fullname(node, true, '_', "VERSION"sv);
        macros["version_string"]     = fullname(node, true, '_', "VERSION"sv, "STRING"sv);
        macros["begin_enum"]         = fullname(node, true, '_', "BEGIN"sv, "ENUM"sv);
        macros["end_enum"]           = fullname(node, true, '_', "END"sv, "ENUM"sv);
        macros["flags_enum"]         = fullname(node, true, '_', "FLAGS"sv, "ENUM"sv);
    }

    void renderPlatformHeader(ASTNodeRef node) {
        std::vector brief   = { "Platform-specific definitions and utilities."s };
        std::vector details = { "This header provides cross-platform macros, type definitions, and utility"s,
                                "\n"s,
                                "macros for the "s,
                                apiName(node),
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

        pushImport(node,
                   "platform"sv,
                   false,
                   {
                       { "brief",   std::move(brief)   },
                       { "details", std::move(details) }
        });

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
                        apiName(node),
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
            details = { "This header provides version information for the "s, apiName(node), " library."s };
        }

        pushImport(node,
                   "version"sv,
                   false,
                   {
                       { "brief",   std::move(brief)   },
                       { "details", std::move(details) }
        });

        state.env.render_to(state.out(), findTemplate(semver ? "c_semver.txt"s : "c_strver.txt"s), state.data);
    }

    std::ostream& out() noexcept {
        return state.out();
    }

    void pushImport(ASTNodeRef& node, std::string_view postfix = ""sv, bool main = false, const OverrideDoc& overrideDoc = {}) {
        const auto single = state.data["config"]["single"].get<bool>();
        if (single && !state.includes.empty()) {
            return;
        }
        std::string name;
        std::string includeGuard;
        if (!postfix.empty() && !single) {
            std::string postfixUpper(postfix.data(), postfix.length());
            postfixUpper = convert(postfixUpper, Case::ScreamingSnakeCase);
            name         = fullname(node, false, '-', postfix);
            includeGuard = fullname(node, true, '_', postfixUpper, 'H');
        } else {
            name         = fullname(node, false, '-');
            includeGuard = fullname(node, true, '_', 'H');
        }

        auto import = node.is<IDL_AST_NODE_TYPE_IMPORT>() ? node : node.ctx().emptyNodeRef();

        inja::json data;
        data["include_guard"] = includeGuard;
        data["is_main"]       = main || single;

        state.includes.emplace(state, state.writer.createOutput(filename(name)), import, std::move(data));
        state.mergeImport();
        fillDoc(0, false, node, state.data, !postfix.empty() && !single ? overrideDoc : OverrideDoc{});

        state.env.render_to(state.out(), findTemplate("c_include_guard.txt"), state.data);
    }

    void tryPopImport(ASTNodeRef& node) {
        const auto single = state.data["config"]["single"].get<bool>();
        if (single) {
            return;
        }
        auto currImport = state.includes.top().import;
        auto currParent = node.parent();
        while (currParent && !currParent.is<IDL_AST_NODE_TYPE_IMPORT>()) {
            currParent = currParent.parent();
        }
        if (currParent != currImport) {
            while (currParent != state.includes.top().import) {
                state.data["include_guard_close"] = true;
                state.env.render_to(out(), findTemplate("c_include_guard.txt"), state.data);
                state.includes.pop();
                state.data["include_guard_close"] = false;
                if (!state.includes.empty()) {
                    state.mergeImport();
                }
            }
        }
    }

    static std::filesystem::path filename(const std::string& name) {
        std::filesystem::path filename(convert(name, Case::LispCase));
        filename.replace_extension(".h");
        return filename;
    }

    template <typename... Postfix>
    std::string fullname(ASTNodeRef& node, bool isUpper, char infix, Postfix&&... postfix) {
        std::ostringstream ss;
        ss << node.accept<CName>(state.stdTypes, state.boolType, true, infix, isUpper).str;
        if (sizeof...(postfix) > 0) {
            ((ss << infix, ss << postfix), ...);
        }
        return ss.str();
    }

    std::string apiName(ASTNodeRef& node) {
        std::vector<int> nums{};
        if (auto attr = node.findChild<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>()) {
            auto view = attr.getChilds() | std::views::transform([](const auto& arg) {
                return int(arg->valueInt);
            });
            nums.assign(view.begin(), view.end());
        }
        const auto str = node.name();
        return convert({ str.data(), str.length() }, Case::PascalCase, nums.empty() ? nullptr : &nums);
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
    constexpr auto filters =
        ASTNodeRef::SkipDocs | ASTNodeRef::SkipAttrBuiltins | ASTNodeRef::SkipAttrs | ASTNodeRef::SkipLiterals | ASTNodeRef::SkipTrivials;
    State state{ writer };
    writer.api().acceptRecursive<ASTVisitor>(filters, std::ref(state));
    state.flushImports();
}

} // namespace idl::gen::c
