#include "case_converter.hpp"
#include "visitors.hpp"
#include "writer.hpp"

namespace idl::gen::idl {

namespace {

struct PriorityDocAttr {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>) {
        prior = 0;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>) {
        prior = 1;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>) {
        prior = 2;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>) {
        prior = 3;
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>) {
        prior = 4;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    int prior{ 1000 };
};

struct LiteralPrinter {
    explicit LiteralPrinter(bool addQuotes) noexcept : addQuotes(addQuotes) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_STR>) {
        constexpr std::string_view nonEscapedSymbols = "a-zA-Z0-9_-^.@";

        auto view = node.valueStr();

        std::string valueStr(view.data(), view.length());
        if (!addQuotes) {
            auto validSymbols = true;
            for (auto c : valueStr) {
                auto valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ||
                             c == '-' || c == '^' || c == '.' || c == '@';
                if (!valid) {
                    validSymbols = false;
                    break;
                }
            }
            if (validSymbols) {
                str = valueStr;
                return;
            }
        }
        str = '"' + valueStr + '"';
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_INT>) {
        str = std::to_string(node->valueInt);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_BOOL>) {
        str = node->valueBool ? "true" : "false";
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_FLOAT>) {
        str = std::to_string(node->valueFloat);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    bool addQuotes;
    std::string str;
};

struct AttrArgsGetter {
    explicit AttrArgsGetter(bool origin) noexcept : origin(origin) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_CNAME>) {
        auto view = node.getChilds(origin);
        printArgs(false, view.begin(), view.end());
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_ORDER>) {
        /*auto view = node.getChilds(origin);
        std::vector<ASTNodeRef> childs;
        childs.assign(view.begin(), view.end());
        if ((origin && !childs.empty()) || (!origin && !childs.front()->valueBool)) {
            args = childs.front().accept<LiteralPrinter>(false).str;
        }*/
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_SINGLE>) {
        /*auto view = node.getChilds(origin);
        std::vector<ASTNodeRef> childs;
        childs.assign(view.begin(), view.end());
        if ((origin && !childs.empty()) || (!origin && !childs.front()->valueBool)) {
            args = childs.front().accept<LiteralPrinter>(false).str;
        }*/
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ATTR_VERSION>) {
        auto view = node.getChilds(origin);
        if (origin) {
            printArgs(false, view.begin(), view.end());
        } else {
            std::vector<ASTNodeRef> components{};
            components.assign(view.begin(), view.end());
            if (components.size() == 3) {
                std::ostringstream ss;
                fmt::print(ss, "{}.{}.{}", components[0]->valueInt, components[1]->valueInt, components[2]->valueInt);
                args = ss.str();
            } else {
                printArgs(false, components.begin(), components.end());
            }
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        auto view = node.getChilds(origin);
        printArgs(true, view.begin(), view.end());
    }

    template <typename It>
    void printArgs(bool addQuotes, It begin, It end) {
        std::ostringstream ss;
        auto hasPrev = false;
        for (auto it = begin; it != end; ++it) {
            if (hasPrev) {
                fmt::print(ss, ", ");
            }
            hasPrev = true;
            fmt::print(ss, "{}", (*it).accept<LiteralPrinter>(addQuotes).str);
        }
        args = ss.str();
    }

    bool origin;
    std::string args;
};

struct ASTVisitor {
    struct File {
        Output output;
        ASTNodeRef import;
        int newLines;
    };

    struct State {
        Writer& writer;
        bool origin;
        uint32_t indents;
        uint32_t lineLength;
        std::stack<File> files;
    };

    ASTVisitor(State& state) noexcept : state(state) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        state.files.emplace(state.writer.createOutput(filename(node)), node.ctx().emptyNodeRef(), 0);
        printDecl(node, 0, false);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        printDecl(node, 0, false);
        state.files.emplace(state.writer.createOutput(filename(node)), node, 0);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        printDecl(node, 0, false);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        printDecl(node, 1, true);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    void printDecl(ASTNodeRef& node, int level, bool hasIDoc) {
        popImport(node);
        for (int i = 0; i < newLines() - (hasIDoc ? 1 : 0); ++i) {
            fmt::print(out(), "\n");
        }
        printDoc(node, level, hasIDoc);
        fmt::print(out(), "{:{}}{} {}", "", state.indents * level, token(node), node.name());
        printType(node);
        printValue(node);
        printAttrs(node);
        printIDoc(node, hasIDoc);
    }

    void printType(ASTNodeRef& node) {
        if (auto attrType = node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPE>(state.origin)) {
            if (auto type = node.declType()) {
                if (node.is<IDL_AST_NODE_TYPE_ENUM>() && type.is<IDL_AST_NODE_TYPE_INT_32>() && !state.origin) {
                    return;
                }
                fmt::print(out(), " {{{}}}", type.name());
            }
        }
    }

    void printValue(ASTNodeRef& node) {
    }

    void printAttrs(ASTNodeRef& node) {
        auto attrs = node.attrs() | std::views::filter([](const auto& attr) {
            return attr.is<IDL_AST_NODE_TYPE_ATTR>() && !attr.is<IDL_AST_NODE_TYPE_ATTR_DOC>() &&
                   !attr.is<IDL_AST_NODE_TYPE_ATTR_VALUE>() && !attr.is<IDL_AST_NODE_TYPE_ATTR_TYPE>();
        });

        bool hasPrev = false;
        for (auto attr : attrs) {
            if (!hasPrev) {
                hasPrev = true;
                fmt::print(out(), " [");
            } else {
                fmt::print(out(), ", ");
            }
            printAttr(attr);
        }
        if (hasPrev) {
            fmt::print(out(), "]");
        }
    }

    void printAttr(ASTNodeRef& node) {
        fmt::print(out(), "{}", attrName(node));
        const auto args = node.accept<AttrArgsGetter>(state.origin).args;
        if (!args.empty()) {
            fmt::print(out(), "({})", args);
        }
    }

    void printDoc(ASTNodeRef& node, int level, bool hasIDoc) {
        auto docs = node.attrs() | std::views::filter([hasIDoc](const auto& attr) {
            if (hasIDoc && attr.is<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>()) {
                return false;
            }
            return attr.is<IDL_AST_NODE_TYPE_ATTR_DOC>();
        }) | std::views::transform([](const auto& attr) {
            const auto priority = const_cast<ASTNodeRef&>(attr).accept<PriorityDocAttr>().prior;
            return std::make_pair(priority, attr);
        });
        std::vector<std::pair<int, ASTNodeRef>> sortedDocs{};
        sortedDocs.assign(docs.begin(), docs.end());
        std::ranges::sort(sortedDocs, {}, &std::pair<int, ASTNodeRef>::first);
        for (auto [_, doc] : sortedDocs) {
            fmt::print(out(), "{:{}}", "", state.indents * level);
            printDocMessage(doc, !doc.is<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>());
            fmt::print(out(), "\n");
        }
    }

    void printIDoc(ASTNodeRef& node, bool hasIDoc) {
        if (hasIDoc) {
            if (auto detail = node.findChild<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>()) {
                fmt::print(out(), " @ TODO");
            }
        }
    }

    void printDocMessage(ASTNodeRef& node, bool addAttr) {
        std::string attr = "";
        if (addAttr) {
            attr = " [" + attrName(node) + ']';
        }
        fmt::print(out(), "@ TODO{}", attr);
    }

    void popImport(ASTNodeRef& node) {
        auto currImport = state.files.top().import;
        auto currParent = node.parent();
        while (currParent && !currParent.is<IDL_AST_NODE_TYPE_IMPORT>()) {
            currParent = currParent.parent();
        }
        if (currParent != currImport) {
            fmt::print(out(), "\n");
            while (currParent != state.files.top().import) {
                state.files.pop();
            }
        }
    }

    int newLines() noexcept {
        const auto lines = state.files.top().newLines;

        state.files.top().newLines = 2;
        return lines;
    }

    std::ostream& out() noexcept {
        return state.files.top().output.stream();
    }

    static std::filesystem::path filename(ASTNodeRef& node) {
        auto nameView = node.name();
        std::string name(nameView.data(), nameView.length());
        std::filesystem::path filename(convert(name, Case::LispCase));
        filename.replace_extension(".idl");
        return filename;
    }

    static std::string token(ASTNodeRef& node) {
        return node.accept<DeclToken>().str;
    }

    static std::string attrName(ASTNodeRef& node) {
        return node.accept<AttrName>().str;
    }

    State& state;
};

} // namespace

void generate(Writer& writer) {
    auto origin  = false;
    auto filters = ASTNodeRef::SkipDocs | ASTNodeRef::SkipAttrs | ASTNodeRef::SkipLiterals | ASTNodeRef::SkipTrivials;
    uint32_t indents    = 4;
    uint32_t lineLength = 120;
    if (auto options = writer.options()) {
        indents         = options->getIndents();
        lineLength      = options->getLineLength();
        auto idlOptions = options->getIdlOptions();
        if (idlOptions.prefered_original_style) {
            filters |= ASTNodeRef::OriginalIdl;
            origin = true;
        }
    }
    ASTVisitor::State state{ writer, origin, indents, lineLength };
    writer.api().acceptRecursive<ASTVisitor>(filters, state);
    fmt::print(state.files.top().output.stream(), "\n");
}

} // namespace idl::gen::idl
