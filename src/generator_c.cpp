#include "case_converter.hpp"
#include "fixed_stack.hpp"
#include "idl_resources.hpp"
#include "visitors.hpp"
#include "writer.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace idl::gen::c {

using OverrideDoc = std::vector<std::pair<std::string_view, std::vector<std::string>>>;

namespace {

struct State;

struct Include {
    State& state;
    Output output;
    ASTNodeRef import;
    inja::json data;
};

struct State {
    Writer& writer;
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

struct DoxygenDoc {
    std::string_view doxygen;
    std::variant<ASTNodeRef, std::string_view, std::string> doc;
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

// void printDoxygen(State& state, int level, std::span<DoxygenDoc> docs) {
//     if (docs.empty()) {
//         return;
//     }
//     auto& out          = state.out();
//     const auto indents = state.indents * level;
//     const auto maxLen  = std::ranges::max(docs | std::views::transform([](const auto& doc) {
//         return doc.doxygen.length();
//     }));
//     fmt::print(out, "{:{}}/**\n", "", indents);
//     for (const auto& [doxygen, doc] : docs) {
//         std::vector<std::ostringstream> strings;
//         strings.push_back({});
//         std::visit([&](const auto& arg) {
//             using Type = std::remove_cvref_t<decltype(arg)>;
//             if constexpr (std::is_same_v<std::string, Type> || std::is_same_v<std::string_view, Type>) {
//                 auto hasPrevLine = false;
//                 for (const auto row : arg | std::views::split('\n')) {
//                     std::string_view line(row.begin(), row.end());
//                     if (!line.empty() && line.back() == '\r') {
//                         line = line.substr(0, line.length() - 1);
//                     }
//                     if (hasPrevLine) {
//                         strings.push_back({});
//                     }
//                     strings.back() << line;
//                     hasPrevLine = true;
//                 }
//             } else {
//                 ASTNodeRef node(arg);
//                 for (auto literal : node.getChilds()) {
//                     auto str = literal.template accept<CLiteral>(state.stdTypes, state.boolType).str;
//                     if (str[0] == '\n') {
//                         strings.push_back({});
//                     } else {
//                         strings.back() << str;
//                     }
//                 }
//             }
//         }, doc);
//         if (doxygen.empty()) {
//             fmt::print(out, "{:{}} *\n{:{}} * {:{}} ", "", indents, "", indents, "", 3);
//         } else {
//             fmt::print(out, "{:{}} * @{:{}} ", "", indents, doxygen, maxLen);
//         }
//         for (auto it = strings.begin(); it != strings.end(); ++it) {
//             if (it != strings.begin()) {
//                 fmt::print(out, "{:{}} *  {:{}} ", "", indents, " ", doxygen.empty() ? 2 : maxLen);
//             }
//             fmt::print(out, "{}\n", it->str());
//         }
//     }
//     fmt::print(out, "{:{}} */\n", "", indents);
// }
//
// void fillDocMap(ASTNodeRef node, std::map<int, DoxygenDoc>& docs) {
//     for (auto doc : node.getChilds<IDL_AST_NODE_TYPE_ATTR_DOC>()) {
//         if (!doc.is<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>()) {
//             auto priority   = doc.accept<PriorityDocAttr>().prior;
//             auto doxygenStr = doc.accept<DoxygenVisitor>().str;
//             docs[priority]  = { doxygenStr, doc };
//         }
//     }
// }
//
// template <typename... Node>
// void printDoc(State& state, const OverridDoc& overrideDoc, int level, Node... node) {
//     if (!state.addDoc) {
//         return;
//     }
//
//     std::map<int, DoxygenDoc> sortedDocs;
//     (fillDocMap(node, sortedDocs), ...);
//
//     for (const auto& [node, str] : overrideDoc) {
//         ASTNodeRef ref    = node;
//         auto prior        = ref.accept<PriorityDocAttr>().prior;
//         auto doxygenStr   = ref.accept<DoxygenVisitor>().str;
//         sortedDocs[prior] = { doxygenStr, str };
//     }
//
//     std::vector<DoxygenDoc> doxygen;
//     doxygen.reserve(sortedDocs.size() + 3);
//     if ((node.is<IDL_AST_NODE_TYPE_API, IDL_AST_NODE_TYPE_IMPORT>() || ...)) {
//         const auto filename = state.includes.top().output.filename().string();
//         doxygen.emplace_back("file"sv, filename);
//     }
//     for (auto [_, doc] : sortedDocs) {
//         doxygen.emplace_back(doc.doxygen, doc.doc);
//     }
//
//     const auto isApiNode = ((!node || node.is<IDL_AST_NODE_TYPE_API>()) && ...);
//     // const auto addLicense = state.includes.top().main && isApiNode;
//     // if (addLicense) {
//     //     if (auto license = state.writer.api().findChild<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>()) {
//     //         doxygen.emplace_back(""sv, license);
//     //     }
//     // }
//
//     if (state.addDocGroups) {
//         std::string_view groups[] = { node.accept<DoxygenGroupVisitor>().str... };
//
//         auto group = std::find_if(std::rbegin(groups), std::rend(groups), [](auto a) {
//             return !a.empty();
//         });
//
//         auto it = std::find_if(doxygen.begin(), doxygen.end(), [](const auto& d) {
//             return std::holds_alternative<ASTNodeRef>(d.doc) && std::get<ASTNodeRef>(d.doc).is<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>();
//         });
//
//         doxygen.insert(it, { "ingroup"sv, *group });
//     }
//
//     printDoxygen(state, level, doxygen);
// }

struct ASTVisitor {
    ASTVisitor(State& state) noexcept : state(state) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        auto single = false;
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>()) {
            single = true;
        }
        uint32_t indents  = 4;
        bool addDoc       = false;
        bool addDocGroups = false;
        if (auto options = state.writer.options()) {
            indents       = options->getIndents();
            auto cOptions = options->getCOptions();
            addDoc        = cOptions.add_doc;
            addDocGroups  = cOptions.add_doc_groups;
        }
        if (!addDoc) {
            addDocGroups = false;
        }
        state.data["config"]["single"]            = single;
        state.data["config"]["indents"]           = indents;
        state.data["config"]["add_doc"]           = addDoc;
        state.data["config"]["add_doc_groups"]    = addDocGroups;
        state.data["config"]["add_member_groups"] = addDoc && true;
        state.data["config"]["std_types"]         = stdTypes(node);
        switch (boolType(node)) {
            case IDL_BOOL_TYPE_INT_32:
                state.data["config"]["bool_type"] = "int32";
                break;
            case IDL_BOOL_TYPE_DEFAULT:
            case IDL_BOOL_TYPE_INT_8:
                state.data["config"]["bool_type"] = "int8";
                break;
            case IDL_BOOL_TYPE_STD_BOOL:
                state.data["config"]["bool_type"] = "std_bool";
                break;
        }

        state.env.add_callback("cname", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();

            ASTNodeRef node(ctx, { handle });
            return node.accept<CName>(stdTypes(node), boolType(node)).str;
        });

        state.env.add_callback("ctype", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();

            ASTNodeRef node(ctx, { handle });
            return node.declType().accept<CName>(stdTypes(node), boolType(node)).str;
        });

        state.env.add_callback("cvalue", 1, [this](inja::Arguments& args) {
            auto& ctx   = state.writer.api().ctx();
            auto handle = args.at(0)->get<uint16_t>();
            auto node   = ASTNodeRef(ctx, { handle });
            return node.accept<CValue>(stdTypes(node), boolType(node)).str;
        });

        state.env.add_callback("cliteral", 1, [this](inja::Arguments& args) {
            auto& ctx = state.writer.api().ctx();
            if (args.at(0)->type() == nlohmann::detail::value_t::string) {
                return args.at(0)->get<std::string>();
            } else {
                auto handle = args.at(0)->get<uint16_t>();
                auto node   = ASTNodeRef(ctx, { handle });
                return node.accept<CLiteral>(stdTypes(node), boolType(node)).str;
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
            return std::to_string(args.at(0)->get<int64_t>());
        });

        addMacros(node);
        addPlatformHeader(node);
        addVersionHeader(node);
        pushImport(node, ""sv, true);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        pushImport(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        popImport(node);

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

    void addMacros(ASTNodeRef node) {
        auto& macros                 = state.data["macros"];
        macros["import_api"]         = getFullname(node, false, '_', "api"sv);
        macros["static_build"]       = getFullname(node, true, '_', "STATIC"sv, "BUILD"sv);
        macros["platform_windows"]   = getFullname(node, true, '_', "PLATFORM"sv, "WINDOWS"sv);
        macros["platform_ios"]       = getFullname(node, true, '_', "PLATFORM"sv, "IOS"sv);
        macros["platform_macos"]     = getFullname(node, true, '_', "PLATFORM"sv, "MAC"sv, "OS"sv);
        macros["platform_android"]   = getFullname(node, true, '_', "PLATFORM"sv, "ANDROID"sv);
        macros["platform_linux"]     = getFullname(node, true, '_', "PLATFORM"sv, "LINUX"sv);
        macros["platform_web"]       = getFullname(node, true, '_', "PLATFORM"sv, "WEB"sv);
        macros["constexpr14"]        = getFullname(node, true, '_', "CONSTEXPR"sv, "14"sv);
        macros["version_major"]      = getFullname(node, true, '_', "VERSION"sv, "MAJOR"sv);
        macros["version_minor"]      = getFullname(node, true, '_', "VERSION"sv, "MINOR"sv);
        macros["version_micro"]      = getFullname(node, true, '_', "VERSION"sv, "MICRO"sv);
        macros["version_encode"]     = getFullname(node, true, '_', "VERSION"sv, "ENCODE"sv);
        macros["version_stringize_"] = getFullname(node, true, '_', "VERSION"sv, "STRINGIZE_"sv);
        macros["version_stringize"]  = getFullname(node, true, '_', "VERSION"sv, "STRINGIZE"sv);
        macros["version"]            = getFullname(node, true, '_', "VERSION"sv);
        macros["version_string"]     = getFullname(node, true, '_', "VERSION"sv, "STRING"sv);
        macros["begin_enum"]         = getFullname(node, true, '_', "BEGIN"sv, "ENUM"sv);
        macros["end_enum"]           = getFullname(node, true, '_', "END"sv, "ENUM"sv);
        macros["flags_enum"]         = getFullname(node, true, '_', "FLAGS"sv, "ENUM"sv);
    }

    void addPlatformHeader(ASTNodeRef node) {
    }

    void addVersionHeader(ASTNodeRef node) {
        pushImport(node,
                   "version"sv,
                   false,
                   {
                       { "brief",   { "Library version information and utilities." } },

                       { "details",
                        {
                             "This header provides version information for the ",
                             getFullname(node, false, ' '),
                             " library,",
                             "\n",
                             "including version number components and macros for version comparison",
                             "\n",
                             "and string generation. It supports:",
                             "\n",
                             "  - Major/Minor/Micro version components",
                             "\n",
                             "  - Integer version encoding",
                             "\n",
                             "  - String version generation",
                             "\n",
                         }                                                           }
        });

        struct Semver {
            int64_t major;
            int64_t minor;
            int64_t micro;
        };

        std::variant<Semver, std::string_view> version;
        bool setVersion = false;

        if (const auto options = state.writer.options()) {
            if (const auto ver = options->getVersion()) {
                version    = Semver{ ver->major, ver->minor, ver->micro };
                setVersion = true;
            }
        }
        if (!setVersion) {
            if (auto ver = node.findChild<IDL_AST_NODE_TYPE_ATTR_VERSION>()) {
                auto literals = ver.getChilds<IDL_AST_NODE_TYPE_LITERAL>();
                std::vector<ASTNodeRef> args;
                args.assign(literals.begin(), literals.end());
                if (args.size() == 1 && args[0].is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                    version    = args[0].valueStr();
                    setVersion = true;
                } else if (args.size() == 3 && args[0].is<IDL_AST_NODE_TYPE_LITERAL_INT>() && args[1].is<IDL_AST_NODE_TYPE_LITERAL_INT>() &&
                           args[2].is<IDL_AST_NODE_TYPE_LITERAL_INT>()) {
                    version    = Semver{ args[0]->valueInt, args[1]->valueInt, args[2]->valueInt };
                    setVersion = true;
                }
            }
        }
        if (!setVersion) {
            version = Semver{};
        }

        if (std::holds_alternative<Semver>(version)) {
            const auto& ver     = std::get<Semver>(version);
            state.data["major"] = ver.major;
            state.data["minor"] = ver.minor;
            state.data["micro"] = ver.micro;
            state.env.render_to(state.out(), findTemplate("c_semver.txt"), state.data);
        } else {
        }
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
            name         = getFullname(node, false, '-', postfix);
            includeGuard = getFullname(node, true, '_', postfixUpper, 'H');
        } else {
            name         = getFullname(node, false, '-');
            includeGuard = getFullname(node, true, '_', 'H');
        }
        auto isImport = node.is<IDL_AST_NODE_TYPE_IMPORT>();

        inja::json data;
        data["include_guard"] = includeGuard;
        data["is_main"]       = main || single;

        state.includes.emplace(state, state.writer.createOutput(filename(name)), isImport ? node : node.ctx().emptyNodeRef(), std::move(data));
        state.mergeImport();

        fillDoc(0, false, node, state.data, overrideDoc);

        state.env.render_to(state.out(), findTemplate("c_include_guard.txt"), state.data);
    }

    void popImport(ASTNodeRef& node) {
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
    std::string getFullname(ASTNodeRef& node, bool isUpper, char infix, Postfix&&... postfix) {
        std::ostringstream ss;
        ss << node.accept<CName>(stdTypes(node), boolType(node), true, infix, isUpper).str;
        if (sizeof...(postfix) > 0) {
            ((ss << infix, ss << postfix), ...);
        }
        return ss.str();
    }

    bool stdTypes(ASTNodeRef& /* node */) noexcept {
        bool stdTypes = false;
        if (auto options = state.writer.options()) {
            stdTypes = options->getStdTypes();
        }
        return stdTypes;
    }

    idl_bool_type_t boolType(ASTNodeRef& /* node */) noexcept {
        idl_bool_type_t boolType = IDL_BOOL_TYPE_INT_8;
        if (auto options = state.writer.options()) {
            boolType = options->getBoolType();
            if (boolType == IDL_BOOL_TYPE_DEFAULT) { // TODO
                boolType = IDL_BOOL_TYPE_INT_8;
            }
        }
        return boolType;
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
