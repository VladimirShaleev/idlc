#include "case_converter.hpp"
#include "fixed_stack.hpp"
#include "visitors.hpp"
#include "writer.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace idl::gen::c {

namespace {

struct State;

struct ASTNodeMapCmp {
    bool operator()(ASTNodeRef n1, ASTNodeRef n2) const {
        auto priority1 = n1.accept<PriorityDocAttr>().prior;
        auto priority2 = n2.accept<PriorityDocAttr>().prior;
        return priority1 < priority2;
    }
};

using OverridDoc = std::map<ASTNodeRef, std::string, ASTNodeMapCmp>;

enum Macro {
    ImportApi,
    StaticBuild,
    PlatformWindows,
    PlatformIos,
    PlatformMacOs,
    PlatformAndroid,
    PlatformLinux,
    PlatformWeb,
    Constexpr,
    Constexpr14
};

struct Include {
    State& state;
    Output output;
    ASTNodeRef import;
    std::string includeGuard;
    OverridDoc overrideDoc;
    bool main;

    void printHeader();
    ~Include();
};

struct State {
    Writer& writer;
    uint32_t indents;
    bool addDocGroups;
    bool single;
    std::stack<Include> includes;
    std::array<std::string, magic_enum::enum_count<Macro>()> macros;

    std::ostream& out() noexcept {
        return includes.top().output.stream();
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

void printDoxygen(State& state, int level, std::span<DoxygenDoc> docs) {
    if (docs.empty()) {
        return;
    }
    auto& out          = state.out();
    const auto indents = state.indents * level;
    const auto maxLen  = std::ranges::max(docs | std::views::transform([](const auto& doc) {
        return doc.doxygen.length();
    }));
    fmt::print(out, "{:{}}/**\n", "", indents);
    for (const auto& [doxygen, doc] : docs) {
        std::vector<std::ostringstream> strings;
        strings.push_back({});
        std::visit([&](const auto& arg) {
            using Type = std::remove_cvref_t<decltype(arg)>;
            if constexpr (std::is_same_v<std::string, Type> || std::is_same_v<std::string_view, Type>) {
                auto hasPrevLine = false;
                for (const auto row : arg | std::views::split('\n')) {
                    std::string_view line(row.begin(), row.end());
                    if (!line.empty() && line.back() == '\r') {
                        line = line.substr(0, line.length() - 1);
                    }
                    if (hasPrevLine) {
                        strings.push_back({});
                    }
                    strings.back() << line;
                    hasPrevLine = true;
                }
            } else {
                ASTNodeRef node(arg);
                for (auto literal : node.getChilds()) {
                    auto str = literal.template accept<CLiteral>().str;
                    if (str[0] == '\n') {
                        strings.push_back({});
                    } else {
                        strings.back() << str;
                    }
                }
            }
        }, doc);
        if (doxygen.empty()) {
            fmt::print(out, "{:{}} *\n{:{}} * {:{}} ", "", indents, "", indents, "", 3);
        } else {
            fmt::print(out, "{:{}} * @{:{}} ", "", indents, doxygen, maxLen);
        }
        for (auto it = strings.begin(); it != strings.end(); ++it) {
            if (it != strings.begin()) {
                fmt::print(out, "{:{}} *  {:{}} ", "", indents, " ", doxygen.empty() ? 2 : maxLen);
            }
            fmt::print(out, "{}\n", it->str());
        }
    }
    fmt::print(out, "{:{}} */\n", "", indents);
}

void fillDocMap(ASTNodeRef node, std::map<int, DoxygenDoc>& docs) {
    for (auto doc : node.getChilds<IDL_AST_NODE_TYPE_ATTR_DOC>()) {
        if (!doc.is<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>()) {
            auto priority   = doc.accept<PriorityDocAttr>().prior;
            auto doxygenStr = doc.accept<DoxygenVisitor>().str;
            docs[priority]  = { doxygenStr, doc };
        }
    }
}

template <typename... Node>
void printDoc(State& state, const OverridDoc& overrideDoc, int level, Node... node) {
    std::map<int, DoxygenDoc> sortedDocs;
    (fillDocMap(node, sortedDocs), ...);

    for (const auto& [node, str] : overrideDoc) {
        ASTNodeRef ref    = node;
        auto prior        = ref.accept<PriorityDocAttr>().prior;
        auto doxygenStr   = ref.accept<DoxygenVisitor>().str;
        sortedDocs[prior] = { doxygenStr, str };
    }

    std::vector<DoxygenDoc> doxygen;
    doxygen.reserve(sortedDocs.size() + 3);
    if ((node.is<IDL_AST_NODE_TYPE_API, IDL_AST_NODE_TYPE_IMPORT>() || ...)) {
        const auto filename = state.includes.top().output.filename().string();
        doxygen.emplace_back("file"sv, filename);
    }
    for (auto [_, doc] : sortedDocs) {
        doxygen.emplace_back(doc.doxygen, doc.doc);
    }

    const auto isApiNode  = ((!node || node.is<IDL_AST_NODE_TYPE_API>()) && ...);
    const auto addLicense = state.includes.top().main && isApiNode;
    if (addLicense) {
        if (auto license = state.writer.api().findChild<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>()) {
            doxygen.emplace_back(""sv, license);
        }
    }

    if (state.addDocGroups) {
        std::string_view groups[] = { node.accept<DoxygenGroupVisitor>().str... };

        auto group = std::find_if(std::rbegin(groups), std::rend(groups), [](auto a) {
            return !a.empty();
        });

        auto it = std::find_if(doxygen.begin(), doxygen.end(), [](const auto& d) {
            return std::holds_alternative<ASTNodeRef>(d.doc) &&
                   std::get<ASTNodeRef>(d.doc).is<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>();
        });

        doxygen.insert(it, { "ingroup"sv, *group });
    }

    printDoxygen(state, level, doxygen);
}

void Include::printHeader() {
    printDoc(state, state.includes.top().overrideDoc, 0, state.writer.api(), import);
    fmt::print(output.stream(), "#ifndef {0}\n#define {0}\n", includeGuard);
}

Include::~Include() {
    fmt::print(output.stream(), "\n#endif /* {} */\n", includeGuard);
}

struct ASTVisitor {
    ASTVisitor(State& state) noexcept : state(state) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>()) {
            state.single = true;
        }
        addMacros(node);
        addPlatformFile(node);
        pushImport(node, ""sv, true);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        pushImport(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        popImport(node);

        auto hex    = node.findChild<IDL_AST_NODE_TYPE_ATTR_HEX>();
        auto childs = node.getChilds();
        auto view   = childs | std::views::filter([](const auto& child) {
            return child.is<IDL_AST_NODE_TYPE_CONST>();
        }) | std::views::transform([](auto c) {
            return std::make_pair(c.accept<CName>().str, c);
        });

        std::vector<std::pair<std::string, ASTNodeRef>> consts;
        consts.assign(view.begin(), view.end());

        size_t maxLen = std::ranges::max(consts | std::views::transform([](const auto& c) {
            return c.first.length();
        }));

        fmt::print(out(), "\n");
        printDoc(state, {}, 0, node);
        fmt::print(out(), "typedef enum\n{{\n");
        for (auto it = consts.begin(); it != consts.end(); ++it) {
            const auto& [name, c] = *it;

            auto maxEnum   = c.findChild<IDL_AST_NODE_TYPE_ATTR_BUILTIN_MAX_ENUM>();
            auto isHex     = hex || !!maxEnum;
            auto forward   = c.forwardDecl();
            auto isFirst   = it == consts.begin();
            auto attrValue = c.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>();
            auto args      = attrValue | std::views::transform([isHex](auto arg) {
                return arg.accept<CLiteral>(isHex).str;
            });

            auto values = forward ? std::to_string(evaulateConst(c)) : fmt::format("{}", fmt::join(args, " | "));

            const auto isMultiline = isMultilineDoc(c);
            if (isMultiline) {
                if (!isFirst) {
                    fmt::print(out(), "\n");
                }
                printDoc(state, {}, 1, c);
            }
            fmt::print(out(), "{:{}}{:{}} = {}{}", " ", state.indents, name, maxLen, values, !!maxEnum ? "" : ",");
            if (!isMultiline) {
                printIDoc(c);
            }
            fmt::print(out(), "\n");
        }
        fmt::print(out(), "}} {};\n", node.accept<CName>().str);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    void printIDoc(ASTNodeRef node) {
        auto detail = node.findChild<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>();
        std::ostringstream ss;
        for (auto literal : detail.getChilds()) {
            ss << literal.template accept<CLiteral>().str;
        }
        fmt::print(out(), " /**< {} */", ss.str());
    }

    int64_t evaulateConst(ASTNodeRef node) {
        int64_t result = 0;
        std::queue<ASTNodeRef> args;
        for (auto arg : node.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>()) {
            args.push(arg);
        }
        while (!args.empty()) {
            auto arg = args.front();
            args.pop();

            if (arg.is<IDL_AST_NODE_TYPE_LITERAL_INT>()) {
                result |= arg->valueInt;
            } else if (arg.is<IDL_AST_NODE_TYPE_DECL_REF>()) {
                auto ref = arg.resolveRef();
                assert(ref.is<IDL_AST_NODE_TYPE_CONST>());
                for (auto arg : ref.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>()) {
                    args.push(arg);
                }
            } else {
                assert(!"unreachable code");
            }
        }
        return result;
    }

    void addMacros(ASTNodeRef node) {
        state.macros[ImportApi]       = includeFullame(node, false, '_', "api"sv);
        state.macros[StaticBuild]     = includeFullame(node, true, '_', "STATIC"sv, "BUILD"sv);
        state.macros[PlatformWindows] = includeFullame(node, true, '_', "PLATFORM"sv, "WINDOWS"sv);
        state.macros[PlatformIos]     = includeFullame(node, true, '_', "PLATFORM"sv, "IOS"sv);
        state.macros[PlatformMacOs]   = includeFullame(node, true, '_', "PLATFORM"sv, "MAC"sv, "OS"sv);
        state.macros[PlatformAndroid] = includeFullame(node, true, '_', "PLATFORM"sv, "ANDROID"sv);
        state.macros[PlatformLinux]   = includeFullame(node, true, '_', "PLATFORM"sv, "LINUX"sv);
        state.macros[PlatformWeb]     = includeFullame(node, true, '_', "PLATFORM"sv, "WEB"sv);
        state.macros[Constexpr]       = includeFullame(node, true, '_', "CONSTEXPR"sv);
        state.macros[Constexpr14]     = includeFullame(node, true, '_', "CONSTEXPR"sv, "14"sv);
    }

    void addPlatformFile(ASTNodeRef node) {
        auto brief   = ASTNodeRef::byType<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(node.ctx());
        auto detial  = ASTNodeRef::byType<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>(node.ctx());
        auto apiName = state.writer.api().name();

        OverridDoc doc;
        doc[brief] = "Platform-specific definitions and utilities."s;
        doc[detial] = R"(This header provides cross-platform macros, type definitions, and utility
macros for the )"s + std::string(apiName.data(), apiName.length()) +
                      R"( library. It handles:
  - Platform detection (Windows, macOS, iOS, Android, Linux, Web)
  - Symbol visibility control (DLL import/export on Windows)
  - C/C++ interoperability
  - Type definitions for consistent data sizes across platforms
  - Bit flag operations for enumerations (C++ only).
)"s;

        pushImport(node, "platform", false, doc);
    }

    std::ostream& out() noexcept {
        return state.out();
    }

    void pushImport(ASTNodeRef& node,
                    std::string_view postfix      = ""sv,
                    bool main                     = false,
                    const OverridDoc& overrideDoc = {}) {
        if (state.single && !state.includes.empty()) {
            return;
        }
        std::string name;
        std::string includeGuard;
        if (!postfix.empty() && !state.single) {
            std::string postfixUpper(postfix.data(), postfix.length());
            postfixUpper = convert(postfixUpper, Case::ScreamingSnakeCase);
            name         = includeFullame(node, false, '-', postfix);
            includeGuard = includeFullame(node, true, '_', postfixUpper, 'H');
        } else {
            name         = includeFullame(node, false, '-');
            includeGuard = includeFullame(node, true, '_', 'H');
        }
        auto isImport = node.is<IDL_AST_NODE_TYPE_IMPORT>();
        state.includes.emplace(state,
                               state.writer.createOutput(filename(name)),
                               isImport ? node : node.ctx().emptyNodeRef(),
                               includeGuard,
                               overrideDoc,
                               main);
        state.includes.top().printHeader();
    }

    void popImport(ASTNodeRef& node) {
        if (state.single) {
            return;
        }
        auto currImport = state.includes.top().import;
        auto currParent = node.parent();
        while (currParent && !currParent.is<IDL_AST_NODE_TYPE_IMPORT>()) {
            currParent = currParent.parent();
        }
        if (currParent != currImport) {
            while (currParent != state.includes.top().import) {
                state.includes.pop();
            }
        }
    }

    [[nodiscard]] static bool isMultilineDoc(const ASTNodeRef& node) {
        auto attrDocDetail = node.findChild<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>();
        if (attrDocDetail.multilineDoc()) {
            return true;
        }
        for (auto arg : attrDocDetail.getChilds()) {
            if (arg.is<IDL_AST_NODE_TYPE_LITERAL_STR>() && arg.valueStr()[0] == '\n') {
                return true;
            }
        }
        return false;
    }

    static std::filesystem::path filename(const std::string& name) {
        std::filesystem::path filename(convert(name, Case::LispCase));
        filename.replace_extension(".h");
        return filename;
    }

    template <typename... Postfix>
    static std::string includeFullame(ASTNodeRef& node, bool isUpper, char infix, Postfix&&... postfix) {
        std::ostringstream ss;
        ss << node.accept<CName>(true, infix, isUpper).str;
        if (sizeof...(postfix) > 0) {
            ((ss << infix, ss << postfix), ...);
        }
        return ss.str();
    }

    State& state;
};

} // namespace

void generate(Writer& writer) {
    constexpr auto filters = ASTNodeRef::SkipDocs | ASTNodeRef::SkipAttrBuiltins | ASTNodeRef::SkipAttrs |
                             ASTNodeRef::SkipLiterals | ASTNodeRef::SkipTrivials;
    uint32_t indents       = 4;
    bool addDocGroups      = false;
    if (auto options = writer.options()) {
        indents       = options->getIndents();
        auto cOptions = options->getCOptions();
        addDocGroups  = cOptions.add_doc_groups;
    }
    State state{ writer, indents, addDocGroups };
    writer.api().acceptRecursive<ASTVisitor>(filters, std::ref(state));
}

} // namespace idl::gen::c
