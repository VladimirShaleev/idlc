#ifndef IDL_VISITORS_HPP
#define IDL_VISITORS_HPP

#include "context.hpp"

namespace idl {

struct CNativeType {
    explicit CNativeType(idl_bool_type_t boolType) noexcept : boolType(boolType) {
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_VOID>) noexcept {
        str = "void";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_DATA>) noexcept {
        str = "void*";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_CHAR>) noexcept {
        str = "char";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_STR>) noexcept {
        str = "const char*";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_BOOL>) noexcept {
        switch (boolType) {
            case IDL_BOOL_TYPE_INT_32:
                str = "int32_t";
                break;
            case IDL_BOOL_TYPE_DEFAULT:
            case IDL_BOOL_TYPE_INT_8:
                str = "int8_t";
                break;
            case IDL_BOOL_TYPE_STD_BOOL:
                str = "bool";
                break;
            default:
                assert(!"unreachable code");
        }
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_INT_8>) noexcept {
        str = "int8_t";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_UINT_8>) noexcept {
        str = "uint8_t";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_INT_16>) noexcept {
        str = "int16_t";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_UINT_16>) noexcept {
        str = "uint16_t";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_INT_32>) noexcept {
        str = "int32_t";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_UINT_32>) noexcept {
        str = "uint32_t";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_INT_64>) noexcept {
        str = "int64_t";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_UINT_64>) noexcept {
        str = "uint64_t";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_FLOAT_32>) noexcept {
        str = "float";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_FLOAT_64>) noexcept {
        str = "double";
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"unreachable code");
    }

    idl_bool_type_t boolType;
    std::string_view str;
};

struct CName {
    struct Convention {
        bool includeImports;
        bool fullname;
        Case caseConvention;
        std::string_view prefix;
        std::string_view postfix;
        std::string_view enumFlags;
        std::string_view constBit;
    };

    struct Settings {
        bool stdTypes;
        idl_bool_type_t boolType;
        std::array<Convention, IDL_AST_NODE_TYPE_FLOAT_64 + 1> conventions;
    };

    explicit CName(const Settings& settings) noexcept : settings(settings) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_ENUM> tag) {
        const auto& conv = settings.conventions[tag.type];
        if (node.findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>()) {
            str = cname(node, conv.includeImports, conv.fullname, conv.caseConvention, conv.prefix, conv.enumFlags, conv.postfix);
        } else {
            str = cname(node, conv.includeImports, conv.fullname, conv.caseConvention, conv.prefix, conv.postfix);
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST> tag) {
        const auto& conv = settings.conventions[tag.type];
        if (node.parent().findChild<IDL_AST_NODE_TYPE_ATTR_FLAGS>()) {
            str = cname(node, conv.includeImports, conv.fullname, conv.caseConvention, conv.prefix, conv.constBit, conv.postfix);
        } else {
            str = cname(node, conv.includeImports, conv.fullname, conv.caseConvention, conv.prefix, conv.postfix);
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type> tag) noexcept {
        if (settings.stdTypes && node.is<IDL_AST_NODE_TYPE_TRIVIAL_TYPE>()) {
            const auto native = node.accept<CNativeType>(settings.boolType).str;
            str.assign(native.data(), native.length());
        } else {
            const auto& conv = settings.conventions[tag.type];

            str = cname(node, conv.includeImports, conv.fullname, conv.caseConvention, conv.prefix, conv.postfix);
        }
    }

    std::string cnameDecl(ASTNodeRef& decl, Case caseConvention) {
        assert(decl.is<IDL_AST_NODE_TYPE_DECL>());
        if (auto attr = decl.findChild<IDL_AST_NODE_TYPE_ATTR_CNAME>()) {
            auto str = decl.ctx().getNodeRef(attr->child).valueStr();
            std::string result{ str.data(), str.length() };
            return caseConvention == Case::ScreamingLispCase || caseConvention == Case::ScreamingSnakeCase ? upper(result) : result;
        }
        std::vector<int> nums{};
        if (auto attr = decl.findChild<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>()) {
            auto view = attr.getChilds() | std::views::transform([](const auto& arg) {
                return int(arg->valueInt);
            });
            nums.assign(view.begin(), view.end());
        }
        auto str = decl.name();
        return convert({ str.data(), str.length() }, caseConvention, nums.empty() ? nullptr : &nums);
    }

    template <typename... Args>
    std::string cname(ASTNodeRef decl, bool includeImports, bool fullname, Case caseConvention, std::string_view prefix, Args&&... postfix) {
        FixedStack<std::string, 20> names;
        while (decl) {
            if (decl.is<IDL_AST_NODE_TYPE_DECL>()) {
                if (includeImports || !decl.is<IDL_AST_NODE_TYPE_IMPORT>()) {
                    names.push(cnameDecl(decl, caseConvention));
                }
            }
            if (!fullname) {
                break;
            }
            decl = decl.parent();
        }
        const auto separator = getSeparator(caseConvention);

        std::ostringstream ss;
        ss << prefix;

        auto first = true;
        while (!names.empty()) {
            if (!first) {
                ss << separator;
            }
            ss << names.top();
            names.pop();
            first = false;
        }
        ((ss << postfix), ...);
        return ss.str();
    }

    static std::string_view getSeparator(Case caseConvention) noexcept {
        switch (caseConvention) {
            case Case::LispCase:
                [[fallthrough]];
            case Case::ScreamingLispCase:
                return "-";
            case Case::CamelCase:
                [[fallthrough]];
            case Case::PascalCase:
                return "";
            case Case::SnakeCase:
                [[fallthrough]];
            case Case::ScreamingSnakeCase:
                return "_";
            case Case::SpaceCase:
                return " ";
            default:
                assert(!"unreachable code");
                return "";
        }
    }

    std::string str;
    const Settings& settings;
};

struct CLiteral {
    explicit CLiteral(const CName::Settings& settings, bool hex = false) noexcept : settings(settings), hex(hex) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_STR>) {
        auto view = node.valueStr();
        str.assign(view.begin(), view.end());
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_LITERAL_INT>) {
        auto type = node.ctx().emptyNodeRef();
        auto curr = node;
        while (curr) {
            if (type = curr.declType(); type) {
                break;
            }
            curr = curr.parent();
        }
        if (!type) {
            type = node.ctx().getTrivial<IDL_AST_NODE_TYPE_INT_64>();
        }

        auto isUnsigned = false;
        if (type.is<IDL_AST_NODE_TYPE_INTEGER_TYPE>()) {
            isUnsigned = type.is<IDL_AST_NODE_TYPE_UINT_8>() || type.is<IDL_AST_NODE_TYPE_UINT_16>() || type.is<IDL_AST_NODE_TYPE_UINT_32>() ||
                         type.is<IDL_AST_NODE_TYPE_UINT_64>();
        }

        if (isUnsigned) {
            str = hex ? fmt::format("{:#X}", uint64_t(node->valueInt)) : std::to_string(uint64_t(node->valueInt));
        } else {
            str = hex ? fmt::format("{:#X}", node->valueInt) : std::to_string(node->valueInt);
        }
        if (hex) {
            if (auto pos = str.find_first_of('X'); pos != std::string::npos) {
                str[pos] = 'x';
            }
        }
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_DECL_REF>) {
        str = node.resolveRef().accept<CName>(std::cref(settings)).str;
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"literal is missing");
    }

    std::string str;
    const CName::Settings& settings;
    bool hex;
};

struct CValue {
    explicit CValue(const CName::Settings& settings) noexcept : settings(settings) {
    }

    void visit(ASTNodeRef& node, Tag<IDL_AST_NODE_TYPE_CONST>) {
        auto value   = node.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>();
        auto hex     = !!node.parent().findChild<IDL_AST_NODE_TYPE_ATTR_HEX>();
        auto maxEnum = !!node.findChild<IDL_AST_NODE_TYPE_ATTR_BUILTIN_MAX_ENUM>();
        if (node.forwardDecl() || maxEnum) {
            auto literal      = ASTNodeRef::byType<IDL_AST_NODE_TYPE_LITERAL_INT>(node.ctx());
            literal->valueInt = value->valueInt;
            literal->parent   = value->parent;
            str               = literal.accept<CLiteral>(std::cref(settings), hex || maxEnum).str;
        } else {
            auto hasPrev = false;
            std::ostringstream ss;
            for (auto child : value) {
                if (hasPrev) {
                    ss << " | ";
                }
                ss << child.accept<CLiteral>(std::cref(settings), hex).str;
                hasPrev = true;
            }
            str = ss.str();
        }
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef& node, Tag<Type>) noexcept {
        auto value = node.findChild<IDL_AST_NODE_TYPE_ATTR_VALUE>();

        str = value.accept<CLiteral>(std::cref(settings), false).str;
    }

    std::string str;
    const CName::Settings& settings;
};

struct PriorityDocAttr {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF> tag) {
        setPriority(tag);
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL> tag) {
        setPriority(tag);
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR> tag) {
        setPriority(tag);
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT> tag) {
        setPriority(tag);
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE> tag) {
        setPriority(tag);
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
    }

    template <ASTNodeType Type>
    void setPriority(Tag<Type> tag) noexcept {
        prior = int(decltype(tag)::type);
    }

    int prior{ 1000 };
};

struct DeclToken {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_API>) {
        str = "api";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ENUM>) {
        str = "enum";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_CONST>) {
        str = "const";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_STRUCT>) {
        str = "struct";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_FIELD>) {
        str = "field";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_FUNC>) {
        str = "func";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ARG>) {
        str = "arg";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_IMPORT>) {
        str = "import";
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"declaration name is missing");
    }

    std::string_view str;
};

struct AttrName {
    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_TOKENIZER>) {
        str = "tokenizer";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_ARRAY>) {
        str = "array";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_SINGLE>) {
        str = "single";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_STD_TYPES>) {
        str = "stdtypes";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_BOOL_TYPE>) {
        str = "booltype";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_VERSION>) {
        str = "version";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR>) {
        str = "author";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT>) {
        str = "copyright";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE>) {
        str = "license";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_FLAGS>) {
        str = "flags";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_HEX>) {
        str = "hex";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF>) {
        str = "brief";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL>) {
        str = "detail";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_VALUE>) {
        str = "value";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_TYPE>) {
        str = "type";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_CNAME>) {
        str = "cname";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_REF>) {
        str = "ref";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_CONST>) {
        str = "const";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_OPTIONAL>) {
        str = "optional";
    }

    void visit(ASTNodeRef&, Tag<IDL_AST_NODE_TYPE_ATTR_BUILTIN_MAX_ENUM>) {
        str = "maxenum";
    }

    template <ASTNodeType Type>
    void visit(ASTNodeRef&, Tag<Type>) noexcept {
        assert(!"attribute name is missing");
    }

    std::string_view str;
};

} // namespace idl

#endif
