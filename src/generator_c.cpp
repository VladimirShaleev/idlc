#include "case_converter.hpp"
#include "fixed_stack.hpp"
#include "visitors.hpp"
#include "writer.hpp"

using namespace std::string_view_literals;

namespace idl::gen::c {

namespace {

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

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>) {
        str = "copyright"sv;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    std::string_view str;
};

struct ASTVisitor {
    struct Include {
        Output output;
        ASTNodeRef import;
        std::string includeGuard;
        bool hasPrevDecl;
    };

    struct State {
        Writer& writer;
        uint32_t indents;
        bool addDocGroups;
        bool single;
        std::stack<Include>& includes;
    };

    ASTVisitor(State& state) noexcept : state(state) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        pushImport(node);
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>()) {
            state.single = true;
        }
        popImport(node);
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

        if (state.includes.top().hasPrevDecl) {
            fmt::print(out(), "\n");
        }
        printDoc(node, 0, "enums");
        fmt::print(out(), "typedef enum\n{{\n");
        for (auto it = consts.begin(); it != consts.end(); ++it) {
            const auto& [name, c] = *it;

            auto maxEnum   = c.findChild<IDL_AST_NODE_TYPE_ATTR_MAX_ENUM>();
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
                printDoc(c, 1, "");
            }
            fmt::print(out(), "{:{}}{:{}} = {}{}", " ", state.indents, name, maxLen, values, !!maxEnum ? "" : ",");
            if (!isMultiline) {
                printIDoc(c);
            }
            fmt::print(out(), "\n");
        }
        fmt::print(out(), "}} {};\n", node.accept<CName>().str);
        state.includes.top().hasPrevDecl = true;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    void printDoc(ASTNodeRef node, int level, std::string_view group) {
        auto attrs = node.attrs();
        auto view  = attrs | std::views::filter([](const auto& attr) {
            return attr.template is<IDL_AST_NODE_TYPE_ATTR_DOC>();
        }) | std::views::transform([](auto doc) {
            auto priority = doc.accept<PriorityDocAttr>().prior;
            return std::make_pair(priority, doc);
        });
        std::vector<std::pair<int, ASTNodeRef>> sortedDocs{};
        sortedDocs.assign(view.begin(), view.end());
        std::ranges::sort(sortedDocs, {}, &std::pair<int, ASTNodeRef>::first);

        auto docView = sortedDocs | std::views::transform([](auto doc) {
            return std::make_pair(doc.second.accept<DoxygenVisitor>().str, doc.second);
        });

        std::vector<std::pair<std::string, std::variant<ASTNodeRef, std::string_view>>> docs;
        docs.assign(docView.begin(), docView.end());

        if (state.addDocGroups && !group.empty()) {
            docs.emplace_back("ingroup", group);
        }
        size_t maxLen = docs.empty() ? 0 : std::ranges::max(docs | std::views::transform([](const auto& c) {
            return c.first.length();
        }));

        const auto indents = state.indents * level;
        fmt::print(out(), "{:{}}/**\n", "", indents);
        for (const auto& [doxygen, doc] : docs) {
            std::vector<std::ostringstream> strings;
            strings.push_back({});
            std::visit([&](const auto& arg) {
                using Type = std::remove_cvref_t<decltype(arg)>;
                if constexpr (std::is_same_v<std::string_view, Type>) {
                    strings.back() << arg;
                } else {
                    ASTNodeRef node(arg);
                    for (auto literal : node) {
                        auto str = literal.template accept<CLiteral>().str;
                        if (str[0] == '\n') {
                            strings.push_back({});
                        } else {
                            strings.back() << str;
                        }
                    }
                }
            }, doc);
            fmt::print(out(), "{:{}} * @{:{}} ", "", indents, doxygen, maxLen);
            for (auto it = strings.begin(); it != strings.end(); ++it) {
                if (it != strings.begin()) {
                    fmt::print(out(), "{:{}} *  {:{}} ", "", indents, " ", maxLen);
                }
                fmt::print(out(), "{}\n", it->str());
            }
        }
        fmt::print(out(), "{:{}} */\n", "", indents);
    }

    void printIDoc(ASTNodeRef node) {
        auto detail = node.findChild<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>();
        std::ostringstream ss;
        for (auto literal : detail) {
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

    std::ostream& out() noexcept {
        return state.includes.top().output.stream();
    }

    void pushImport(ASTNodeRef& node) {
        auto isImport = node.is<IDL_AST_NODE_TYPE_IMPORT>();
        if (isImport && state.single) {
            return;
        }
        const auto name = includeFullame(node);
        state.includes.emplace(state.writer.createOutput(filename(name)),
                               isImport ? node : node.ctx().emptyNodeRef(),
                               includeGuard(name),
                               false);
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
        for (auto arg : attrDocDetail) {
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

    static std::string includeGuard(const std::string& name) {
        return convert(name, Case::ScreamingSnakeCase) + "_H";
    }

    static std::string includeFullame(ASTNodeRef& node) {
        return node.accept<CName>(true, '-').str;
    }

    State& state;
};

} // namespace

void generate(Writer& writer) {
    constexpr auto filters = ASTNodeRef::SkipDocs | ASTNodeRef::SkipAttrBuiltins | ASTNodeRef::SkipAttrs |
                             ASTNodeRef::SkipLiterals | ASTNodeRef::SkipTrivials;
    uint32_t indents       = 4;
    bool addDocGroups      = false;
    std::stack<ASTVisitor::Include> includes;
    if (auto options = writer.options()) {
        indents       = options->getIndents();
        auto cOptions = options->getCOptions();
        addDocGroups  = cOptions.add_doc_groups;
    }
    ASTVisitor::State state{ writer, indents, addDocGroups, false, std::ref(includes) };
    writer.api().acceptRecursive<ASTVisitor>(filters, std::ref(state));
}

} // namespace idl::gen::c
