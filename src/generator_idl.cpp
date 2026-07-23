#include "case_converter.hpp"
#include "fixed_stack.hpp"
#include "visitors.hpp"
#include "writer.hpp"

using namespace std::string_literals;

namespace idl::gen::idl {

namespace {

std::string escapeStr(std::string_view str, bool multiLine) {
    std::ostringstream ss;
    bool escapeSpaces = false;
    for (size_t i = 0; i < str.length(); ++i) {
        auto c  = str[i];
        auto nc = i + 1 < str.length() ? str[i + 1] : '\0';
        if (c == '{' || c == '}' || c == '[' || c == ']') {
            ss << '\\' << c;
        } else if (c == ' ' && nc == ' ' && !multiLine) {
            ss << "\\s";
            escapeSpaces = true;
        } else if (escapeSpaces) {
            ss << "\\s";
        } else if (c == '\t' && !multiLine) {
            ss << "\\t";
        } else if (c == '\\') {
            ss << "\\\\";
        } else if (c == '\n' && !multiLine) {
            ss << "\\n";
        } else if (c == '`' && multiLine) {
            ss << "\\`";
        } else {
            ss << c;
        }
    }
    return ss.str();
}

std::string refName(ASTNodeRef path, ASTNodeRef exclude) {
    FixedStack<ASTNodeRef, 20> pathStack;
    FixedStack<ASTNodeRef, 20> excludeStack;
    auto fillStack = [](auto& stack, auto node) {
        while (node) {
            if (node.template is<IDL_AST_NODE_TYPE_DECL>() && !node.template is<IDL_AST_NODE_TYPE_IMPORT>()) {
                stack.push(node);
            }
            node = node.parent();
        }
    };
    fillStack(pathStack, path);
    fillStack(excludeStack, exclude);

    while (!pathStack.empty() && !excludeStack.empty()) {
        auto pathStr    = pathStack.top()->name.name;
        auto excludeStr = excludeStack.top()->name.name;
        if (pathStr.handle != excludeStr.handle) {
            break;
        }
        pathStack.pop();
        excludeStack.pop();
    }
    auto hasPrev = false;
    std::ostringstream ss;
    while (!pathStack.empty()) {
        if (hasPrev) {
            ss << '.';
        }
        hasPrev = true;
        ss << pathStack.top().name();
        pathStack.pop();
    }
    return ss.str();
}

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
    explicit LiteralPrinter(bool addQuotes, ASTNodeRef currDepth) noexcept :
        addQuotes(addQuotes),
        currDepth(currDepth) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_STR>) {
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
        str = fmt::format("{:g}", node->valueFloat);
        if (str.find_last_of('.') == std::string::npos) {
            str += ".0"s;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_DECL_REF>) {
        str = refName(node.resolveRef(), currDepth);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"unknown literal type");
    }

    bool addQuotes;
    ASTNodeRef currDepth;
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
        auto view = node.getChilds(origin);
        if (origin) {
            printArgs(false, view.begin(), view.end());
        } else {
            std::ostringstream ss;
            auto hasPrev = false;
            for (auto arg : view) {
                if (hasPrev) {
                    ss << '-';
                }
                if (const auto value = arg->valueInt; value < 0) {
                    ss << '^' << (-value);
                } else {
                    ss << value;
                }
                hasPrev = true;
            }
            args = ss.str();
        }
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
            fmt::print(ss, "{}", (*it).template accept<LiteralPrinter>(addQuotes, (*it).parent()).str);
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
        bool single;
    };

    ASTVisitor(State& state) noexcept : state(state) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_API>) {
        pushImport(node);
        printDecl(node, 0, false);
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_SINGLE>()) {
            state.single = true;
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        if (!state.single) {
            printDecl(node, 0, false);
        }
        pushImport(node);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        printDecl(node, 0, false);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        printDecl(node, 1, true);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_STRUCT>) {
        printDecl(node, 0, false);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FIELD>) {
        printDecl(node, 1, true);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        printDecl(node, 0, false);
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ARG>) {
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
        printIDoc(node, level, hasIDoc);
    }

    void printType(ASTNodeRef& node) {
        if (auto attrType = node.findChild<IDL_AST_NODE_TYPE_ATTR_TYPE>(state.origin)) {
            if (auto type = node.declType()) {
                if (node.is<IDL_AST_NODE_TYPE_ENUM>() && type.is<IDL_AST_NODE_TYPE_INT_32>() && !state.origin) {
                    return;
                }
                if (node.is<IDL_AST_NODE_TYPE_FUNC>() && type.is<IDL_AST_NODE_TYPE_VOID>() && !state.origin) {
                    return;
                }
                fmt::print(out(), " {{{}}}", type.name());
            }
        }
    }

    void printValue(ASTNodeRef& node) {
        if (auto attrValue = node.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>(state.origin)) {
            auto hasPrev = false;
            for (auto value : attrValue.getChilds(state.origin)) {
                if (value.addedByCompiler() && value.parent().parent().is<IDL_AST_NODE_TYPE_CONST>()) {
                    continue;
                }
                auto valStr = value.accept<LiteralPrinter>(true, node.parent()).str;
                fmt::print(out(), "{}{}", hasPrev ? ", " : " : ", valStr);
                hasPrev = true;
            }
        }
    }

    void printAttrs(ASTNodeRef& node) {
        auto attrs = node.attrs(state.origin) | std::views::filter([](const auto& attr) {
            return attr.template is<IDL_AST_NODE_TYPE_ATTR>() && !attr.template is<IDL_AST_NODE_TYPE_ATTR_DOC>() &&
                   !attr.template is<IDL_AST_NODE_TYPE_ATTR_VALUE>() &&
                   !attr.template is<IDL_AST_NODE_TYPE_ATTR_TYPE>();
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
            if (hasIDoc && attr.template is<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>()) {
                return false;
            }
            return attr.template is<IDL_AST_NODE_TYPE_ATTR_DOC>();
        }) | std::views::transform([](const auto& attr) {
            const auto priority = const_cast<ASTNodeRef&>(attr).accept<PriorityDocAttr>().prior;
            return std::make_pair(priority, attr);
        });
        std::vector<std::pair<int, ASTNodeRef>> sortedDocs{};
        sortedDocs.assign(docs.begin(), docs.end());
        std::ranges::sort(sortedDocs, {}, &std::pair<int, ASTNodeRef>::first);
        for (auto [_, doc] : sortedDocs) {
            fmt::print(out(), "{:{}}", "", state.indents * level);
            printDocMessage(doc, level, !doc.is<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>(), false);
            fmt::print(out(), "\n");
        }
    }

    void printIDoc(ASTNodeRef& node, int level, bool hasIDoc) {
        if (hasIDoc) {
            if (auto detail = node.findChild<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>()) {
                fmt::print(out(), " ");
                printDocMessage(detail, level, false, true);
            }
        }
    }

    void printDocMessage(ASTNodeRef& node, int level, bool addAttr, bool idoc) {
        auto isMultiline = (node->flags & IDL_AST_NODE_STATE_MULTILINE_DOC_BIT) == IDL_AST_NODE_STATE_MULTILINE_DOC_BIT;
        auto isNewLine   = isMultiline;
        fmt::print(out(), "{:{}}@ {}", "", idoc ? 0 : state.indents * level, isMultiline ? "```\n" : "");
        for (auto data : node) {
            if (isMultiline && isNewLine) {
                fmt::print(out(), "{:{}}", "", state.indents * (level + 1));
            }
            if (data.is<IDL_AST_NODE_TYPE_LITERAL_STR>()) {
                auto str = data.valueStr();
                fmt::print(out(), "{}", escapeStr(str, isMultiline));
                isNewLine = !str.empty() && str[0] == '\n';
            } else if (data.is<IDL_AST_NODE_TYPE_DECL_REF>()) {
                fmt::print(out(), "{{{}}}", refName(data.resolveRef(), node.parent().parent()));
                isNewLine = false;
            }
        }
        if (isMultiline) {
            fmt::print(out(), "```");
        }
        if (addAttr) {
            fmt::print(out(), " [{}]", attrName(node));
        }
    }

    void pushImport(ASTNodeRef& node) {
        auto isImport = node.is<IDL_AST_NODE_TYPE_IMPORT>();
        if (isImport && state.single) {
            return;
        }
        state.files.emplace(state.writer.createOutput(filename(node)), isImport ? node : node.ctx().emptyNodeRef(), 0);
    }

    void popImport(ASTNodeRef& node) {
        if (state.single) {
            return;
        }
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

    static std::string_view token(ASTNodeRef& node) {
        return node.accept<DeclToken>().str;
    }

    static std::string_view attrName(ASTNodeRef& node) {
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
