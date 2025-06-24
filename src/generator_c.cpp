
#include "case_converter.hpp"
#include "context.hpp"

using namespace idl;

struct Header {
    std::ostream& stream;
    std::unique_ptr<std::ofstream> fstream;
    std::unique_ptr<std::ostringstream> sstream;
    std::string filename;
    std::string includeGuard;
    bool externC;
    idl_write_callback_t writer;
    idl_data_t writerData;

    friend std::ostream& operator<<(std::ostream& os, const Header& header) {
        return os << header.filename;
    }
};

struct DocRef : Visitor {
    void visit(ASTYear* node) override {
        str = std::to_string(node->value);
    }

    void visit(ASTMajor* node) override {
        str = std::to_string(node->value);
    }

    void visit(ASTMinor* node) override {
        str = std::to_string(node->value);
    }

    void visit(ASTMicro* node) override {
        str = std::to_string(node->value);
    }

    void visit(ASTDocBool* node) override {
        str = node->value ? "TRUE" : "FALSE";
    }

    void visit(ASTEnumConst* node) override {
        idl::CName name;
        node->accept(name);
        str = "::" + name.str;
    }

    void visit(ASTField* node) override {
        idl::CName name;
        node->parent->accept(name);
        str = name.str + "::";
        node->accept(name);
        str += name.str;
    }

    void visit(ASTMethod* node) override {
        idl::CName name;
        node->accept(name);
        str = "::" + name.str;
    }

    void visit(ASTArg* node) override {
        idl::CName name;
        node->accept(name);
        str = '*' + name.str + '*';
    }

    void discarded(ASTNode* node) override {
        idl::CName name;
        node->accept(name);
        str = name.str;
    }

    std::string str;
};

static std::string headerStr(idl::Context& ctx, std::string postfix = "") {
    return convert(ctx.api()->name, Case::LispCase) + (postfix.length() ? "-" + lower(postfix) : "") + ".h";
}

static std::string includeGuardStr(idl::Context& ctx, std::string postfix = "") {
    return convert(ctx.api()->name, Case::ScreamingSnakeCase) + (postfix.length() ? "_" + upper(postfix) : "") + "_H";
}

static Header createHeader(idl::Context& ctx,
                           const std::filesystem::path& out,
                           const std::string& postfix,
                           bool externC,
                           idl_write_callback_t writer,
                           idl_data_t writerData) {
    std::filesystem::create_directories(out);
    auto header = out / headerStr(ctx, postfix);
    auto guard  = includeGuardStr(ctx, postfix);
    if (writer) {
        auto stream = std::make_unique<std::ostringstream>();
        auto ptr    = stream.get();
        return { *ptr, nullptr, std::move(stream), header.filename().string(), guard, externC, writer, writerData };
    } else {
        auto stream = std::make_unique<std::ofstream>(std::ofstream(header));
        if (stream->fail()) {
            idl::err<IDL_STATUS_E2067>(ctx.api()->location, header.string());
        }
        auto ptr = stream.get();
        return { *ptr, std::move(stream), nullptr, header.filename().string(), guard, externC };
    }
}

static std::string getApiPrefix(idl::Context& ctx, bool upper) {
    return convert(ctx.api()->name, upper ? Case::ScreamingSnakeCase : Case::SnakeCase);
}

static ASTDecl* getDeclType(ASTDecl* decl) noexcept {
    assert(decl->findAttr<ASTAttrType>() != nullptr);
    return decl->findAttr<ASTAttrType>()->type->decl;
}

static std::string getDeclTypeCName(ASTDecl* decl) {
    idl::CName name;
    getDeclType(decl)->accept(name);
    return name.str;
}

static std::string getDeclCName(ASTDecl* decl, int removePostfix = 0) {
    idl::CName name;
    decl->accept(name);
    return removePostfix == 0 ? name.str : name.str.substr(0, name.str.length() - removePostfix);
}

static ASTLiteral* getDeclValueLiteral(ASTDecl* decl) {
    assert(decl->findAttr<ASTAttrValue>() != nullptr);
    return decl->findAttr<ASTAttrValue>()->value;
}

static std::string getDeclValue(ASTDecl* decl, bool hexOut) {
    std::string value;
    auto literal = getDeclValueLiteral(decl);
    if (auto integer = literal->as<ASTLiteralInt>()) {
        if (hexOut) {
            auto hexLength = integer->value == 0 ? 1 : size_t(std::log2(integer->value) / 4 + 1);
            auto width     = hexLength % 2 == 0 ? hexLength : hexLength + 1;
            value          = fmt::format("0x{:0{}x}", integer->value, width);
        } else {
            value = fmt::format("{}", integer->value);
        }
    } else if (auto refs = literal->as<ASTLiteralConsts>()) {
        idl::CName name;
        for (auto ref : refs->decls) {
            ref->decl->accept(name);
            value += value.length() > 0 ? " | " + name.str : name.str;
        }
    } else {
        assert(!"unreachable code");
    }
    return value;
}

static bool isConstDecl(ASTDecl* decl) noexcept {
    return decl->findAttr<ASTAttrConst>() != nullptr;
}

static bool isRefDecl(ASTDecl* decl) noexcept {
    return decl->findAttr<ASTAttrRef>() != nullptr;
}

static bool isOutDecl(ASTDecl* decl) noexcept {
    return decl->findAttr<ASTAttrOut>() != nullptr;
}

static std::string getType(ASTDecl* field, bool isReturn = false) {
    auto type = getDeclTypeCName(field);
    if ((!isReturn && isConstDecl(field)) || (isReturn && isConstDecl(field) && isRefDecl(field))) {
        type.insert(0, "const ");
    }
    if (isRefDecl(field) || isOutDecl(field)) {
        type += '*';
    }
    return type;
}

static std::pair<std::string, std::string> getTypeAndName(ASTDecl* field) {
    auto name = getDeclCName(field);
    if (auto arr = field->findAttr<ASTAttrArray>(); arr && !arr->ref) {
        name += arr->size > 1 ? '[' + std::to_string(arr->size) + ']' : "";
    }
    return { getType(field), name };
}

template <typename... Includes>
static void beginHeader(idl::Context& ctx, Header& header, const Includes&... includes) {
    fmt::println(header.stream, "#ifndef {}", header.includeGuard);
    fmt::println(header.stream, "#define {}", header.includeGuard);
    fmt::println(header.stream, "");
    if (sizeof...(includes) > 0) {
        auto stum = []() {};
        ((std::string(includes).length() > 0
              ? fmt::println(header.stream, "#include \"{}\"", headerStr(ctx, std::string(includes)))
              : stum()),
         ...);
        fmt::println(header.stream, "");
    }
    if (header.externC) {
        fmt::println(header.stream, "{}_BEGIN", convert(ctx.api()->name, Case::ScreamingSnakeCase));
        fmt::println(header.stream, "");
    }
}

static void endHeader(idl::Context& ctx, Header& header) {
    if (header.externC) {
        fmt::println(header.stream, "{}_END", convert(ctx.api()->name, Case::ScreamingSnakeCase));
        fmt::println(header.stream, "");
    }
    fmt::println(header.stream, "#endif /* {} */", header.includeGuard);
    if (header.writer) {
        const std::string data = header.sstream->str();
        idl_source_t source{ header.filename.c_str(), data.c_str(), (idl_uint32_t) data.length() + 1 };
        header.writer(&source, header.writerData);
    }
}

static void generateDocField(Header& header,
                             const std::vector<ASTNode*>& nodes,
                             size_t indents,
                             const std::string& prefix,
                             bool inlineDoc = false) {
    bool first = true;
    for (auto node : nodes) {
        if (auto str = node->as<ASTLiteralStr>()) {
            fmt::print(header.stream, "{}", str->value);
            if (str->value == "\n") {
                fmt::print(header.stream, " *{:<{}}{}", ' ', indents, prefix);
            }
        } else if (auto ref = node->as<ASTDeclRef>()) {
            DocRef docRef;
            ref->decl->accept(docRef);
            fmt::print(header.stream, "{}", docRef.str);
        } else {
            assert(!"unreachable code");
        }
        first = false;
    }
    if (!inlineDoc) {
        fmt::println(header.stream, "");
    }
}

static void generateDoc(Header& header,
                        ASTDecl* node,
                        bool printLicense                = false,
                        ASTFile* fileDecl                = nullptr,
                        const std::vector<ASTArg*>* args = nullptr) {
    if (!node->doc) {
        return;
    }

    size_t maxLength = 0;
    auto calcLength  = [&maxLength](std::string_view field, const auto& nodes) {
        if (!nodes.empty()) {
            if (field.length() > maxLength) {
                maxLength = field.length();
            }
        }
    };

    auto printDocField = [&header, &maxLength](std::string_view field,
                                               const std::vector<ASTNode*>& nodes,
                                               const std::string prefix  = "",
                                               const std::string argName = "") {
        if (!nodes.empty()) {
            auto at = field.length() > 0 ? "@" : "";
            fmt::print(header.stream,
                       " * {}{:<{}}{}{}",
                       at,
                       field,
                       maxLength + (field.length() > 0 ? 1 : 0),
                       prefix,
                       argName.empty() ? "" : argName + " ");
            generateDocField(header, nodes, maxLength + (field.length() > 0 ? 3 : 1), prefix);
        }
    };

    auto printDocFields = [&header, &printDocField](std::string_view field,
                                                    const std::vector<std::vector<ASTNode*>>& nodes,
                                                    bool parblock = false) {
        for (const auto& node : nodes) {
            if (parblock && nodes.size() > 1) {
                fmt::println(header.stream, " * @parblock");
            }
            printDocField(field, node);
            if (parblock && nodes.size() > 1) {
                fmt::println(header.stream, " * @endparblock");
            }
        }
    };

    std::string_view file       = "file";
    std::string_view author     = node->doc->authors.size() > 1 ? "authors" : "author";
    std::string_view brief      = "brief";
    std::string_view details    = "details";
    std::string_view paramin    = "param[in]";
    std::string_view paramout   = "param[out]";
    std::string_view paraminout = "param[in,out]";
    std::string_view ret        = "return";
    std::string_view copyright  = "copyright";
    std::string_view note       = "note";
    std::string_view warning    = "warning";
    std::string_view sa         = "sa";

    auto& briefNodes  = fileDecl && !fileDecl->doc->brief.empty() ? fileDecl->doc->brief : node->doc->brief;
    auto& detailNodes = fileDecl && !fileDecl->doc->detail.empty() ? fileDecl->doc->detail : node->doc->detail;
    calcLength(author, node->doc->authors);
    calcLength(brief, briefNodes);
    calcLength(details, detailNodes);
    calcLength(ret, node->doc->ret);
    calcLength(copyright, node->doc->copyright);
    calcLength(note, node->doc->note);
    calcLength(warning, node->doc->warn);
    calcLength(sa, node->doc->see);
    if (args) {
        for (auto arg : *args) {
            const auto isIn  = arg->findAttr<ASTAttrIn>() != nullptr;
            const auto isOut = arg->findAttr<ASTAttrOut>() != nullptr;
            if (isIn && isOut) {
                calcLength(paraminout, arg->doc->detail);
            } else if (isIn) {
                calcLength(paramin, arg->doc->detail);
            } else if (isOut) {
                calcLength(paramout, arg->doc->detail);
            } else {
                assert(!"unreachable code");
            }
        }
    }
    fmt::println(header.stream, "/**");
    if (node->is<ASTApi>()) {
        if (file.length() > maxLength) {
            maxLength = file.length();
        }
        fmt::println(header.stream, " * @{:<{}} {}", file, maxLength, header.filename);
    }
    printDocField(brief, briefNodes);
    printDocField(details, detailNodes);
    if (args) {
        for (auto arg : *args) {
            const auto isIn    = arg->findAttr<ASTAttrIn>() != nullptr;
            const auto isOut   = arg->findAttr<ASTAttrOut>() != nullptr;
            const auto argName = getDeclCName(arg);
            if (isIn && isOut) {
                printDocField(paraminout, arg->doc->detail, "", argName);
            } else if (isIn) {
                printDocField(paramin, arg->doc->detail, "", argName);
            } else if (isOut) {
                printDocField(paramout, arg->doc->detail, "", argName);
            }
        }
    }
    printDocField(ret, node->doc->ret);
    printDocFields(author, node->doc->authors);
    printDocFields(note, node->doc->note, true);
    printDocFields(warning, node->doc->warn, true);
    printDocFields(sa, node->doc->see);
    printDocField(copyright, node->doc->copyright);
    if (printLicense && !node->doc->copyright.empty()) {
        maxLength = 0;
        fmt::println(header.stream, " *");
        // add 4 spaces to the beginning of lines so doxygen will consider it a block
        std::string indent4 = "    ";
        printDocField("", node->doc->license, indent4);
    }
    fmt::println(header.stream, " */");
}

static void generateInlineDoc(Header& header, ASTDecl* node, bool includeBrief = false, bool briefOnly = false) {
    if (node->doc && !node->doc->detail.empty()) {
        fmt::print(header.stream, " /**< ");
        if (includeBrief && !node->doc->brief.empty()) {
            generateDocField(header, node->doc->brief, 0, "", true);
            if (auto str = node->doc->brief.back()->as<ASTLiteralStr>()) {
                if (!std::ispunct(str->value.back())) {
                    fmt::print(header.stream, ".");
                }
                if (!briefOnly) {
                    fmt::print(header.stream, " ");
                }
            }
        }
        if (!briefOnly) {
            generateDocField(header, node->doc->detail, 0, "", true);
        }
        fmt::print(header.stream, " */");
    }
}

struct DeclGenerator : Visitor {
    DeclGenerator(Header& h, idl::Context& context) noexcept : header(h), ctx(context) {
    }

    void visit(ASTEnum* node) override {
        const auto isHexOut = node->findAttr<ASTAttrHex>() != nullptr;
        std::vector<std::tuple<std::string, std::string, ASTDecl*>> consts;
        consts.reserve(node->consts.size());
        size_t maxLength = 0;
        for (auto ec : node->consts) {
            consts.emplace_back(getDeclCName(ec), getDeclValue(ec, isHexOut) + ',', ec);
            if (std::get<0>(consts.back()).length() > maxLength) {
                maxLength = std::get<0>(consts.back()).length();
            }
        }
        ASTEnumConst ec{};
        ec.parent = node;
        ec.name   = "MaxEnum";
        auto name = getDeclCName(&ec, node->findAttr<ASTAttrFlags>() ? 4 : 0);
        consts.emplace_back(name, "0x7FFFFFFF", nullptr);
        if (name.length() > maxLength) {
            maxLength = name.length();
        }
        generateDoc(header, node);
        fmt::println(header.stream, "typedef enum");
        fmt::println(header.stream, "{{");
        for (const auto& [key, value, decl] : consts) {
            fmt::print(header.stream, "{:<{}}{:<{}} = {}", ' ', 4, key, maxLength, value);
            if (decl) {
                generateInlineDoc(header, decl);
            } else {
                fmt::print(header.stream, " /**< Max value of enum (not used) */");
            }
            fmt::println(header.stream, "");
        }
        name = getDeclCName(node);
        fmt::println(header.stream, "}} {};", name);
        if (node->findAttr<ASTAttrFlags>()) {
            auto API = getApiPrefix(ctx, true);
            fmt::println(header.stream, "{}_FLAGS({})", API, name);
        }
        fmt::println(header.stream, "");
    }

    void visit(ASTStruct* node) override {
        if (!node->findAttr<ASTAttrHandle>()) {
            size_t maxLength = 0;
            std::vector<std::tuple<std::string, std::string, ASTDecl*>> typeNames;
            for (auto field : node->fields) {
                const auto [type, name] = getTypeAndName(field);
                typeNames.emplace_back(type, name, field);
                if (type.length() > maxLength) {
                    maxLength = type.length();
                }
            }
            generateDoc(header, node);
            fmt::println(header.stream, "typedef struct");
            fmt::println(header.stream, "{{");
            for (const auto& [key, value, decl] : typeNames) {
                fmt::print(header.stream, "{:<{}}{:<{}} {};", ' ', 4, key, maxLength, value);
                generateInlineDoc(header, decl);
                fmt::println(header.stream, "");
            }
            fmt::println(header.stream, "}} {};", getDeclCName(node));
            fmt::println(header.stream, "");
        }
    }

    void visit(ASTFunc* node) override {
        printFunc(node, node->args);
    }

    void visit(ASTCallback* node) override {
        generateDoc(header, node, false, nullptr, &node->args);
        const auto decl = fmt::format("(*{})(", getDeclCName(node));
        fmt::println(header.stream, "typedef {}", getType(node, true));
        fmt::print(header.stream, "{}", decl);
        if (node->args.empty()) {
            fmt::print(header.stream, "void");
        } else {
            for (size_t i = 0; i < node->args.size(); ++i) {
                const auto [argType, argName] = getTypeAndName(node->args[i]);
                if (i == 0) {
                    fmt::print(header.stream, "{} {}", argType, argName);
                } else {
                    fmt::print(header.stream, "{:>{}} {}", argType, decl.length() + argType.length(), argName);
                }
                if (i + 1 < node->args.size()) {
                    fmt::println(header.stream, ",");
                }
            }
        }
        fmt::println(header.stream, ");");
        fmt::println(header.stream, "");
    }

    void visit(ASTMethod* node) override {
        printFunc(node, node->args);
    }

    void printFunc(ASTDecl* decl, const std::vector<ASTArg*>& args) {
        generateDoc(header, decl, false, nullptr, &args);
        auto api       = getApiPrefix(ctx, false);
        auto importApi = api + "_api";
        fmt::println(header.stream, "{} {}", importApi, getType(decl, true));
        auto declStr = fmt::format("{}(", getDeclCName(decl));
        fmt::print(header.stream, "{}", declStr);
        if (args.empty()) {
            fmt::print(header.stream, "void");
        } else {
            for (size_t i = 0; i < args.size(); ++i) {
                const auto [typeStr, nameStr] = getTypeAndName(args[i]);
                if (i == 0) {
                    fmt::print(header.stream, "{} {}", typeStr, nameStr);
                } else {
                    fmt::print(header.stream, "{:>{}} {}", typeStr, declStr.length() + typeStr.length(), nameStr);
                }
                if (i + 1 < args.size()) {
                    fmt::println(header.stream, ",");
                }
            }
        }
        fmt::println(header.stream, ");");
        fmt::println(header.stream, "");
    }

    Header& header;
    idl::Context& ctx;
};

static void generateVersion(idl::Context& ctx,
                            const std::filesystem::path& out,
                            idl_write_callback_t writer,
                            idl_data_t writerData) {
    auto API    = getApiPrefix(ctx, true);
    auto header = createHeader(ctx, out, "version", false, writer, writerData);
    auto major  = 0;
    auto minor  = 0;
    auto micro  = 0;
    if (ctx.apiVersion()) {
        major = ctx.apiVersion().value().major;
        minor = ctx.apiVersion().value().minor;
        micro = ctx.apiVersion().value().micro;
    } else if (auto version = ctx.api()->findAttr<ASTAttrVersion>()) {
        major = version->major;
        minor = version->minor;
        micro = version->micro;
    }

    constexpr auto tmp = R"(/**
 * @name  Version Components
 * @brief Individual components of the library version
 * @{{
 */

/**
 * @brief Major version number (API-breaking changes)
 * @sa    {API}_VERSION
 * @sa    {API}_VERSION_STRING
 */
#define {API}_VERSION_MAJOR {major}

/**
 * @brief Minor version number (backwards-compatible additions)
 * @sa    {API}_VERSION
 * @sa    {API}_VERSION_STRING
 */
#define {API}_VERSION_MINOR {minor}

/**
 * @brief Micro version number (bug fixes and patches)
 * @sa    {API}_VERSION
 * @sa    {API}_VERSION_STRING
 */
#define {API}_VERSION_MICRO {micro}

/** @}} */

/**
 * @name  Version Utilities
 * @brief Macros for working with version numbers
 * @{{
 */

/**
 * @brief     Encodes version components into a single integer
 * @details   Combines major, minor, and micro versions into a 32-bit value:
 *            - Bits 24-31: Major version
 *            - Bits 16-23: Minor version
 *            - Bits 0-15: Micro version
 * @param[in] major Major version number
 * @param[in] minor Minor version number
 * @param[in] micro Micro version number
 * @return    Encoded version as unsigned long
 * @sa        {API}_VERSION
 */
#define {API}_VERSION_ENCODE(major, minor, micro) (((unsigned long) major) << 16 | (minor) << 8 | (micro))

/**
 * @brief     Internal macro for string version generation
 * @details   Helper macro that stringizes version components (e.g., {major}, {minor}, {micro} -> "{major}.{minor}.{micro}")
 * @param[in] major Major version number
 * @param[in] minor Minor version number
 * @param[in] micro Micro version number
 * @return    Stringified version
 * @note      For internal use only
 * @private
 */
#define {API}_VERSION_STRINGIZE_(major, minor, micro) #major "." #minor "." #micro

/**
 * @def       {API}_VERSION_STRINGIZE
 * @brief     Creates version string from components
 * @details   Generates a string literal from version components (e.g., {major}, {minor}, {micro} -> "{major}.{minor}.{micro}")
 * @param[in] major Major version number
 * @param[in] minor Minor version number
 * @param[in] micro Micro version number
 * @return    Stringified version
 * @sa        {API}_VERSION_STRING
 */
#define {API}_VERSION_STRINGIZE(major, minor, micro)  {API}_VERSION_STRINGIZE_(major, minor, micro)

/** @}} */

/**
 * @name  Current Version
 * @brief Macros representing the current library version
 * @{{
 */

/**
 * @brief   Encoded library version as integer
 * @details Combined version value suitable for numeric comparisons.
 *          Use #{API}_VERSION_STRING for human-readable format.
 * @sa      {API}_VERSION_STRING
 */
#define {API}_VERSION {API}_VERSION_ENCODE( \
    {API}_VERSION_MAJOR, \
    {API}_VERSION_MINOR, \
    {API}_VERSION_MICRO)

/**
 * @def     {API}_VERSION_STRING
 * @brief   Library version as human-readable string
 * @details Version string in "MAJOR.MINOR.MICRO" format (e.g., "{major}.{minor}.{micro}").
 *          Use #{API}_VERSION for numeric comparisons.
 * @sa      {API}_VERSION
 */
#define {API}_VERSION_STRING {API}_VERSION_STRINGIZE( \
    {API}_VERSION_MAJOR, \
    {API}_VERSION_MINOR, \
    {API}_VERSION_MICRO)

/** @}} */
)";
    std::vector<ASTLiteralStr> strings;
    strings.reserve(20);
    auto addDocField = [&strings](std::vector<std::string>&& data) {
        std::vector<ASTNode*> nodes;
        for (const auto& str : data) {
            strings.push_back({});
            strings.back().value = str;
            nodes.push_back(&strings.back());
        }
        return nodes;
    };

    ASTDoc doc{};
    doc.brief  = addDocField({ "Library version information and utilities." });
    doc.detail = addDocField({ "This header provides version information for the " + ctx.api()->name + " library,",
                               "\n",
                               "including version number components and macros for version comparison",
                               "\n",
                               "and string generation. It supports:",
                               "\n",
                               "- Major/Minor/Micro version components",
                               "\n",
                               "- Integer version encoding",
                               "\n",
                               "- String version generation",
                               "\n" });
    ASTFile file{};
    file.name = "version";
    file.doc  = &doc;
    generateDoc(header, ctx.api(), false, &file);
    beginHeader(ctx, header);
    fmt::println(header.stream,
                 tmp,
                 fmt::arg("API", API),
                 fmt::arg("major", major),
                 fmt::arg("minor", minor),
                 fmt::arg("micro", micro));
    endHeader(ctx, header);
}

static void generatePlatform(idl::Context& ctx,
                             const std::filesystem::path& out,
                             idl_write_callback_t writer,
                             idl_data_t writerData) {
    auto API       = getApiPrefix(ctx, true);
    auto api       = getApiPrefix(ctx, false);
    auto importAPI = api + "_api";
    auto header    = createHeader(ctx, out, "platform", false, writer, writerData);
    auto intType   = std::string();

    size_t maxLength     = 0;
    size_t maxLengthType = 0;
    std::vector<std::tuple<std::string, std::string, ASTDecl*>> trivialTypes;
    ctx.filter<ASTBuiltinType>([&trivialTypes, &intType, &maxLength, &maxLengthType](ASTBuiltinType* node) {
        if (!node->as<ASTVoid>()) {
            idl::CName name;
            node->accept(name);
            trivialTypes.emplace_back(name.native, name.str, node);
            if (name.native.length() > maxLength) {
                maxLength = name.native.length();
            }
            if (name.str.length() > maxLengthType) {
                maxLengthType = name.str.length();
            }
            if (node->as<ASTInt32>()) {
                intType = name.str;
            }
        }
    });

    std::vector<ASTLiteralStr> strings;
    strings.reserve(20);
    auto addDocField = [&strings](std::vector<std::string>&& data) {
        std::vector<ASTNode*> nodes;
        for (const auto& str : data) {
            strings.push_back({});
            strings.back().value = str;
            nodes.push_back(&strings.back());
        }
        return nodes;
    };

    ASTDoc doc{};
    doc.brief  = addDocField({ "Platform-specific definitions and utilities." });
    doc.detail = addDocField({ "This header provides cross-platform macros, type definitions, and utility",
                               "\n",
                               "macros for the " + ctx.api()->name + " library. It handles:",
                               "\n",
                               "- Platform detection (Windows, macOS, iOS, Android, Linux, Web)",
                               "\n",
                               "- Symbol visibility control (DLL import/export on Windows)",
                               "\n",
                               "- C/C++ interoperability",
                               "\n",
                               "- Type definitions for consistent data sizes across platforms",
                               "\n",
                               "- Bit flag operations for enumerations (C++ only).",
                               "\n" });
    ASTFile file{};
    file.name = "platform";
    file.doc  = &doc;
    generateDoc(header, ctx.api(), false, &file);
    beginHeader(ctx, header);
    fmt::println(header.stream, "/**");
    fmt::println(header.stream, " * @def     {}_BEGIN", API);
    fmt::println(header.stream, " * @brief   Begins a C-linkage declaration block.");
    fmt::println(header.stream,
                 " * @details In C++, expands to `extern \"C\" {{` to ensure C-compatible symbol naming.");
    fmt::println(header.stream, " *          In pure C environments, expands to nothing.");
    fmt::println(header.stream, " * @sa      {}_END", API);
    fmt::println(header.stream, " *");
    fmt::println(header.stream, " */");
    fmt::println(header.stream, "");
    fmt::println(header.stream, "/**");
    fmt::println(header.stream, " * @def     {}_END", API);
    fmt::println(header.stream, " * @brief   Ends a C-linkage declaration block.");
    fmt::println(header.stream, " * @details Closes the scope opened by #{}_BEGIN.", API);
    fmt::println(header.stream, " * @sa      {}_BEGIN", API);
    fmt::println(header.stream, " *");
    fmt::println(header.stream, " */");
    fmt::println(header.stream, "");
    fmt::println(header.stream, "#ifdef __cplusplus");
    fmt::println(header.stream, "# define {}_BEGIN extern \"C\" {{", API);
    fmt::println(header.stream, "# define {}_END   }}", API);
    fmt::println(header.stream, "#else");
    fmt::println(header.stream, "# define {}_BEGIN", API);
    fmt::println(header.stream, "# define {}_END", API);
    fmt::println(header.stream, "#endif");
    fmt::println(header.stream, "");
    fmt::println(header.stream, "/**");
    fmt::println(header.stream, " * @def     {}", importAPI);
    fmt::println(header.stream, " * @brief   Controls symbol visibility for shared library builds.");
    fmt::println(header.stream,
                 " * @details This macro is used to control symbol visibility when building or using the library.");
    fmt::println(header.stream,
                 " *          On Windows (**MSVC**) with dynamic linking (non-static build), it expands to "
                 "`__declspec(dllimport)`.");
    fmt::println(header.stream,
                 " *          In all other cases (static builds or non-Windows platforms), it expands to nothing.");
    fmt::println(header.stream, " *          This allows proper importing of symbols from DLLs on Windows platforms.");
    fmt::println(header.stream, " * @note    Define `{}_STATIC_BUILD` for static library configuration.", API);
    fmt::println(header.stream, " */");
    fmt::println(header.stream, "");
    fmt::println(header.stream, "#ifndef {}", importAPI);
    fmt::println(header.stream, "# if defined(_MSC_VER) && !defined({}_STATIC_BUILD)", API);
    fmt::println(header.stream, "#  define {} __declspec(dllimport)", importAPI);
    fmt::println(header.stream, "# else");
    fmt::println(header.stream, "#  define {}", importAPI);
    fmt::println(header.stream, "# endif");
    fmt::println(header.stream, "#endif");
    fmt::println(header.stream, "");
    fmt::println(header.stream, "#if defined(_WIN32) && !defined({}_PLATFORM_WINDOWS)", API);
    fmt::println(header.stream, "# define {}_PLATFORM_WINDOWS", API);
    fmt::println(header.stream, "#elif defined(__APPLE__)");
    fmt::println(header.stream, "# include <TargetConditionals.h>");
    fmt::println(header.stream, "# include <unistd.h>");
    fmt::println(header.stream, "# if TARGET_OS_IPHONE && !defined({}_PLATFORM_IOS)", API);
    fmt::println(header.stream, "#  define {}_PLATFORM_IOS", API);
    fmt::println(header.stream, "# elif TARGET_IPHONE_SIMULATOR && !defined({}_PLATFORM_IOS)", API);
    fmt::println(header.stream, "#  define {}_PLATFORM_IOS", API);
    fmt::println(header.stream, "# elif TARGET_OS_MAC && !defined({}_PLATFORM_MAC_OS)", API);
    fmt::println(header.stream, "#  define {}_PLATFORM_MAC_OS", API);
    fmt::println(header.stream, "# else");
    fmt::println(header.stream, "#  error unsupported Apple platform");
    fmt::println(header.stream, "# endif");
    fmt::println(header.stream, "#elif defined(__ANDROID__) && !defined({}_PLATFORM_ANDROID)", API);
    fmt::println(header.stream, "# define {}_PLATFORM_ANDROID", API);
    fmt::println(header.stream, "#elif defined(__linux__) && !defined({}_PLATFORM_LINUX)", API);
    fmt::println(header.stream, "# define {}_PLATFORM_LINUX", API);
    fmt::println(header.stream, "#elif defined(__EMSCRIPTEN__) && !defined({}_PLATFORM_WEB)", API);
    fmt::println(header.stream, "# define {}_PLATFORM_WEB", API);
    fmt::println(header.stream, "#else");
    fmt::println(header.stream, "# error unsupported platform");
    fmt::println(header.stream, "#endif");
    fmt::println(header.stream, "");
    fmt::println(header.stream, "#ifdef __cpp_constexpr");
    fmt::println(header.stream, "#  define {}_CONSTEXPR constexpr", API);
    fmt::println(header.stream, "#  if __cpp_constexpr >= 201304L");
    fmt::println(header.stream, "#    define {}_CONSTEXPR_14 constexpr", API);
    fmt::println(header.stream, "#  else");
    fmt::println(header.stream, "#    define {}_CONSTEXPR_14", API);
    fmt::println(header.stream, "#  endif");
    fmt::println(header.stream, "#else");
    fmt::println(header.stream, "#  define {}_CONSTEXPR", API);
    fmt::println(header.stream, "#  define {}_CONSTEXPR_14", API);
    fmt::println(header.stream, "#endif");
    fmt::println(header.stream, "");
    fmt::println(header.stream, "/**");
    fmt::println(header.stream, " * @name  Platform-independent type definitions");
    fmt::println(header.stream, " * @brief Fixed-size types guaranteed to work across all supported platforms");
    fmt::println(header.stream, " * @{{");
    fmt::println(header.stream, " */");
    fmt::println(header.stream, "#include <stdint.h>");
    for (const auto& [native, type, decl] : trivialTypes) {
        fmt::print(header.stream, "typedef {:<{}} {:<{}}", native, maxLength, type + ';', maxLengthType + 1);
        generateInlineDoc(header, decl);
        fmt::println(header.stream, "");
    }
    fmt::println(header.stream, "/** @}} */");
    fmt::println(header.stream, "");
    constexpr auto tmpFlags = R"(/**
 * @def       {API}_FLAGS
 * @brief     Enables bit flag operations for enumerations (C++ only).
 * @details   Generates overloaded bitwise operators for type-safe flag manipulation:
 *            - Bitwise NOT (~)
 *            - OR (|, |=)
 *            - AND (&, &=)
 *            - XOR (^, ^=)
 * 
 * @param[in] {api}_enum_t Enumeration type to enhance with flag operations
 * @note      Only active in C++ mode. In C, expands to nothing.
 */

#ifdef __cplusplus
# define {API}_FLAGS({api}_enum_t) \
extern "C++" {{ \
inline {API}_CONSTEXPR {api}_enum_t operator~({api}_enum_t lhr) noexcept {{ \
    return static_cast<{api}_enum_t>(~static_cast<{int}>(lhr)); \
}} \
inline {API}_CONSTEXPR {api}_enum_t operator|({api}_enum_t lhr, {api}_enum_t rhs) noexcept {{ \
    return static_cast<{api}_enum_t>(static_cast<{int}>(lhr) | static_cast<{int}>(rhs)); \
}} \
inline {API}_CONSTEXPR {api}_enum_t operator&({api}_enum_t lhr, {api}_enum_t rhs) noexcept {{ \
    return static_cast<{api}_enum_t>(static_cast<{int}>(lhr) & static_cast<{int}>(rhs)); \
}} \
inline {API}_CONSTEXPR {api}_enum_t operator^({api}_enum_t lhr, {api}_enum_t rhs) noexcept {{ \
    return static_cast<{api}_enum_t>(static_cast<{int}>(lhr) ^ static_cast<{int}>(rhs)); \
}} \
inline {API}_CONSTEXPR_14 {api}_enum_t& operator|=({api}_enum_t& lhr, {api}_enum_t rhs) noexcept {{ \
    return lhr = lhr | rhs; \
}} \
inline {API}_CONSTEXPR_14 {api}_enum_t& operator&=({api}_enum_t& lhr, {api}_enum_t rhs) noexcept {{ \
    return lhr = lhr & rhs; \
}} \
inline {API}_CONSTEXPR_14 {api}_enum_t& operator^=({api}_enum_t& lhr, {api}_enum_t rhs) noexcept {{ \
    return lhr = lhr ^ rhs; \
}} \
}}
#else
# define {API}_FLAGS({api}_enum_t)
#endif)";
    fmt::println(header.stream, tmpFlags, fmt::arg("API", API), fmt::arg("api", api), fmt::arg("int", intType));
    fmt::println(header.stream, "");
    fmt::println(header.stream, "/**");
    fmt::println(header.stream, " * @def       {}_TYPE", API);
    fmt::println(header.stream, " * @brief     Declares an opaque handle type.");
    fmt::println(header.stream, " * @details   Creates a typedef for a pointer to an incomplete struct type,");
    fmt::println(header.stream, " *            providing type safety while hiding implementation details.");
    fmt::println(header.stream, " * @param[in] {}_name Base name for the type (suffix `_t` will be added)", api);
    fmt::println(header.stream, " */");
    fmt::println(header.stream, "#define {}_TYPE({}_name) \\", API, api);
    fmt::println(header.stream, "typedef struct _##{}_name* {}_name##_t;", api, api);
    fmt::println(header.stream, "");
    ctx.filter<ASTStruct>([&header, &API, &api](ASTStruct* node) {
        if (node->findAttr<ASTAttrHandle>()) {
            size_t maxLength = 0;
            std::vector<std::pair<std::string, std::string>> typeNames;
            typeNames.reserve(node->fields.size());
            for (auto field : node->fields) {
                typeNames.emplace_back(getTypeAndName(field));
                if (typeNames.back().first.length() > maxLength) {
                    maxLength = typeNames.back().first.length();
                }
            }
            auto name = getDeclCName(node, 2);

            fmt::println(header.stream, "/**");
            fmt::println(header.stream, " * @def       {}_HANDLE", API);
            fmt::println(header.stream, " * @brief     Declares an index-based handle type.");
            fmt::println(header.stream, " * @details   Creates a struct containing an index value, typically used for");
            fmt::println(header.stream, " *            resource handles in API designs that avoid direct pointers.");
            fmt::println(
                header.stream, " * @param[in] {}_name Base name for the handle type (suffix `_h` will be added)", api);
            fmt::println(header.stream, " */");
            fmt::println(header.stream, "#define {}({}_name) \\", upper(name), api);
            fmt::println(header.stream, "typedef struct _##{}_name {{ \\", api);
            for (const auto& [key, value] : typeNames) {
                fmt::println(header.stream, "{:<{}}{:<{}} {}; \\", ' ', 4, key, maxLength, value);
            }
            fmt::println(header.stream, "}} {}_name##_h;", api);
            fmt::println(header.stream, "");
        }
    });
    endHeader(ctx, header);
}

static void generateTypes(idl::Context& ctx,
                          const std::filesystem::path& out,
                          bool hasInterfaces,
                          bool hasHandles,
                          idl_write_callback_t writer,
                          idl_data_t writerData) {
    if (!hasInterfaces && !hasHandles) {
        std::filesystem::remove(out / headerStr(ctx, "types"));
        return;
    }
    auto API    = getApiPrefix(ctx, true);
    auto header = createHeader(ctx, out, "types", true, writer, writerData);

    std::vector<ASTLiteralStr> strings;
    strings.reserve(20);
    auto addDocField = [&strings](std::vector<std::string>&& data) {
        std::vector<ASTNode*> nodes;
        for (const auto& str : data) {
            strings.push_back({});
            strings.back().value = str;
            nodes.push_back(&strings.back());
        }
        return nodes;
    };

    ASTDoc doc{};
    doc.brief = addDocField({ "Core type definitions for the " + ctx.api()->name + " framework." });
    doc.detail =
        addDocField({ "This header defines the fundamental object types and handles used throughout",
                      "\n",
                      "the " + ctx.api()->name + " framework. It provides forward declarations for all major system",
                      "\n",
                      "components using opaque pointer types (#" + API + "_TYPE) and index-based handles",
                      "\n",
                      "(#" + API + "_HANDLE) for better type safety and abstraction." });
    ASTFile file{};
    file.name = "types";
    file.doc  = &doc;
    generateDoc(header, ctx.api(), false, &file);
    beginHeader(ctx, header, "platform");
    if (hasInterfaces) {
        size_t maxLength = 0;
        std::vector<std::pair<std::string, ASTDecl*>> decls;
        ctx.filter<ASTInterface>([&API, &decls, &maxLength](auto node) {
            auto name = getDeclCName(node, 2);
            decls.emplace_back(fmt::format("{}_TYPE({})", API, name), node);
            if (decls.back().first.length() > maxLength) {
                maxLength = decls.back().first.length();
            }
        });
        fmt::println(header.stream, "/**");
        fmt::println(header.stream, " * @name    Opaque Object Types");
        fmt::println(header.stream,
                     " * @brief   Forward declarations for framework objects using opaque pointer types");
        fmt::println(header.stream,
                     " * @details These macros generate typedefs for pointers to incomplete struct types,");
        fmt::println(header.stream,
                     " *          providing type safety while hiding implementation details. Each represents");
        fmt::println(header.stream, " *          a major subsystem in the {} framework.", ctx.api()->name);
        fmt::println(header.stream, " * @sa      {}_TYPE", API);
        fmt::println(header.stream, " * @{{");
        fmt::println(header.stream, " */");
        for (const auto& [str, decl] : decls) {
            fmt::print(header.stream, "{:<{}}", str, maxLength);
            generateInlineDoc(header, decl, true, true);
            fmt::println(header.stream, "");
        }
        fmt::println(header.stream, "/** @}} */");
        fmt::println(header.stream, "");
    }
    if (hasHandles) {
        size_t maxLength = 0;
        std::vector<std::pair<std::string, ASTHandle*>> decls;
        ctx.filter<ASTHandle>([&API, &decls, &maxLength](auto node) {
            auto name = getDeclCName(node, 2);
            decls.emplace_back(fmt::format("{}_HANDLE({})", API, name), node);
            if (decls.back().first.length() > maxLength) {
                maxLength = decls.back().first.length();
            }
        });
        fmt::println(header.stream, "/**");
        fmt::println(header.stream, " * @name    Resource Handles");
        fmt::println(header.stream, " * @brief   Index-based handles");
        fmt::println(header.stream, " * @details These macros generate lightweight handle types,");
        fmt::println(header.stream, " *          using indices rather than pointers for better memory management");
        fmt::println(header.stream, " *          and cross-API compatibility. Each handle contains an internal index.");
        fmt::println(header.stream, " * @sa      {}_HANDLE", API);
        fmt::println(header.stream, " * @{{");
        fmt::println(header.stream, " */");
        for (const auto& [str, decl] : decls) {
            fmt::print(header.stream, "{:<{}}", str, maxLength);
            generateInlineDoc(header, decl);
            fmt::println(header.stream, "");
        }
        fmt::println(header.stream, "/** @}} */");
        fmt::println(header.stream, "");
    }
    endHeader(ctx, header);
}

static void generateFile(idl::Context& ctx,
                         const std::filesystem::path& out,
                         ASTFile* file,
                         ASTFile* prevFile,
                         idl_write_callback_t writer,
                         idl_data_t writerData) {
    auto header = createHeader(ctx, out, convert(file->name, Case::LispCase), true, writer, writerData);
    generateDoc(header, ctx.api(), false, file);
    if (prevFile) {
        beginHeader(ctx, header, convert(prevFile->name, Case::LispCase));
    } else {
        beginHeader(ctx, header, "version", "types");
    }
    DeclGenerator generator(header, ctx);
    for (auto decl : file->decls) {
        decl->accept(generator);
    }
    endHeader(ctx, header);
}

static void generateMain(idl::Context& ctx,
                         const std::filesystem::path& out,
                         ASTFile* prevFile,
                         idl_write_callback_t writer,
                         idl_data_t writerData) {
    auto header = createHeader(ctx, out, "", false, writer, writerData);
    generateDoc(header, ctx.api(), true);
    if (prevFile) {
        beginHeader(ctx, header, convert(prevFile->name, Case::LispCase));
    } else {
        beginHeader(ctx, header, "version", "types");
    }
    DeclGenerator generator(header, ctx);
    ctx.filter<ASTDecl>([&generator](ASTDecl* decl) {
        if (!decl->file) {
            decl->accept(generator);
        }
    });
    endHeader(ctx, header);
}

void generateC(idl::Context& ctx,
               const std::filesystem::path& out,
               idl_write_callback_t writer,
               idl_data_t writerData) {
    auto finish = [](auto) {
        return false;
    };
    auto hasInterfaces = !ctx.filter<ASTInterface>(finish);
    auto hasHandles    = !ctx.filter<ASTHandle>(finish);
    auto hasTypes      = hasInterfaces || hasHandles;
    auto hasEnums      = !ctx.filter<ASTEnum>(finish);
    auto hasCallbacks  = !ctx.filter<ASTCallback>(finish);
    bool hasStructs    = false;
    ctx.filter<ASTStruct>([&hasStructs](ASTStruct* node) {
        if (!node->findAttr<ASTAttrHandle>()) {
            hasStructs = true;
            return false;
        }
        return true;
    });

    generateVersion(ctx, out, writer, writerData);
    generatePlatform(ctx, out, writer, writerData);
    generateTypes(ctx, out, hasInterfaces, hasHandles, writer, writerData);
    ASTFile* prevFile = nullptr;
    for (auto file : ctx.api()->files) {
        generateFile(ctx, out, file, prevFile, writer, writerData);
        prevFile = file;
    }
    generateMain(ctx, out, prevFile, writer, writerData);
}
