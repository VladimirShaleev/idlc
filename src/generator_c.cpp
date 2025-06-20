
#include "case_converter.hpp"
#include "context.hpp"

#include <fmt/ostream.h>
#include <fstream>

struct Header {
    std::ofstream stream;
    std::string filename;
    std::string includeGuard;
    bool externC;

    friend std::ostream& operator<<(std::ostream& os, const Header& header) {
        return os << header.filename;
    }
};

struct DocRef : Visitor {
    void visit(ASTYear* node) override {
        str = std::to_string(node->value);
    }

    void discarded(ASTNode* node) override {
        CName name;
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
                           bool externC) {
    auto header = out / headerStr(ctx, postfix);
    auto guard  = includeGuardStr(ctx, postfix);
    auto stream = std::ofstream(header);
    if (stream.fail()) {
        std::cerr << fmt::format("failed to create file '{}'", header.string()) << std::endl;
        exit(EXIT_FAILURE);
    }
    return { std::move(stream), header.filename().string(), guard, externC };
}

static std::string getApiPrefix(idl::Context& ctx, bool upper) {
    return convert(ctx.api()->name, upper ? Case::ScreamingSnakeCase : Case::SnakeCase);
}

static ASTDecl* getDeclType(ASTDecl* decl) noexcept {
    assert(decl->findAttr<ASTAttrType>() != nullptr);
    return decl->findAttr<ASTAttrType>()->type->decl;
}

static std::string getDeclTypeCName(ASTDecl* decl) {
    CName name;
    getDeclType(decl)->accept(name);
    return name.str;
}

static std::string getDeclCName(ASTDecl* decl, int removePostfix = 0) {
    CName name;
    decl->accept(name);
    return removePostfix == 0 ? name.str : name.str.substr(0, name.str.length() - removePostfix);
}

static ASTLiteral* getDeclValueLiteral(ASTDecl* decl) {
    assert(decl->findAttr<ASTAttrValue>() != nullptr);
    ;
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
        CName name;
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

static std::pair<std::string, std::string> getTypeAndName(ASTDecl* field) {
    auto type = getDeclTypeCName(field);
    if (isConstDecl(field)) {
        type.insert(0, "const ");
    }
    if (isRefDecl(field) || isOutDecl(field)) {
        type += '*';
    }
    auto name = getDeclCName(field);
    if (auto arr = field->findAttr<ASTAttrArray>(); arr && !arr->ref) {
        name += arr->size > 1 ? '[' + std::to_string(arr->size) + ']' : "";
    }
    return { type, name };
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
}

static void generateDocField(Header& header, const std::vector<ASTNode*>& nodes, size_t indents) {
    bool first = true;
    for (auto node : nodes) {
        if (!first) {
            fmt::print(header.stream, " ");
        }
        if (auto str = node->as<ASTLiteralStr>()) {
            fmt::print(header.stream, "{}", str->value);
            if (str->value == "\n") {
                fmt::print(header.stream, " * {:<{}} ", ' ', indents);
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
    fmt::println(header.stream, "");
}

static void generateDoc(idl::Context& ctx, Header& header, ASTDecl* node, bool printLicense = false) {
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

    auto printDocField = [&header, &maxLength](std::string_view field, const std::vector<ASTNode*>& nodes) {
        if (!nodes.empty()) {
            auto at = field.length() > 0 ? "@" : "";
            fmt::print(header.stream, " * {}{:<{}} ", at, field, maxLength);
            generateDocField(header, nodes, maxLength);
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

    std::string_view file      = "file";
    std::string_view author    = node->doc->authors.size() > 1 ? "authors" : "author";
    std::string_view brief     = "brief";
    std::string_view details   = "details";
    std::string_view ret       = "return";
    std::string_view copyright = "copyright";
    std::string_view note      = "note";
    std::string_view warning   = "warning";
    std::string_view sa        = "sa";

    calcLength(author, node->doc->authors);
    calcLength(brief, node->doc->brief);
    calcLength(details, node->doc->detail);
    calcLength(ret, node->doc->ret);
    calcLength(copyright, node->doc->copyright);
    calcLength(note, node->doc->note);
    calcLength(warning, node->doc->warn);
    calcLength(sa, node->doc->see);

    fmt::println(header.stream, "/**");
    if (node->is<ASTApi>()) {
        if (file.length() > maxLength) {
            maxLength = file.length();
        }
        fmt::println(header.stream, " * @{:<{}} {}", file, maxLength, header.filename);
    }
    printDocFields(author, node->doc->authors);
    printDocField(brief, node->doc->brief);
    printDocField(details, node->doc->detail);
    printDocField(ret, node->doc->ret);
    printDocFields(note, node->doc->note, true);
    printDocFields(warning, node->doc->warn, true);
    printDocFields(sa, node->doc->see);
    printDocField(copyright, node->doc->copyright);
    if (printLicense && !node->doc->copyright.empty()) {
        maxLength = 0;
        fmt::println(header.stream, " *");
        printDocField("", node->doc->license);
    }
    fmt::println(header.stream, " */");
}

static void generateVersion(idl::Context& ctx, const std::filesystem::path& out) {
    auto API    = getApiPrefix(ctx, true);
    auto header = createHeader(ctx, out, "version", false);
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

    constexpr auto tmp = R"(#define {API}_VERSION_MAJOR {major}
#define {API}_VERSION_MINOR {minor}
#define {API}_VERSION_MICRO {micro}

#define {API}_VERSION_ENCODE(major, minor, micro) (((unsigned long) major) << 16 | (minor) << 8 | (micro))

#define {API}_VERSION_STRINGIZE_(major, minor, micro) #major "." #minor "." #micro
#define {API}_VERSION_STRINGIZE(major, minor, micro)  {API}_VERSION_STRINGIZE_(major, minor, micro)

#define {API}_VERSION {API}_VERSION_ENCODE( \
    {API}_VERSION_MAJOR, \
    {API}_VERSION_MINOR, \
    {API}_VERSION_MICRO)

#define {API}_VERSION_STRING {API}_VERSION_STRINGIZE( \
    {API}_VERSION_MAJOR, \
    {API}_VERSION_MINOR, \
    {API}_VERSION_MICRO)
)";

    beginHeader(ctx, header);
    fmt::println(header.stream,
                 tmp,
                 fmt::arg("API", API),
                 fmt::arg("major", major),
                 fmt::arg("minor", minor),
                 fmt::arg("micro", micro));
    endHeader(ctx, header);
}

static void generatePlatform(idl::Context& ctx, const std::filesystem::path& out) {
    auto API       = getApiPrefix(ctx, true);
    auto api       = getApiPrefix(ctx, false);
    auto importAPI = api + "_api";
    auto header    = createHeader(ctx, out, "platform", false);
    auto intType   = std::string();

    size_t maxLength = 0;
    std::vector<std::pair<std::string, std::string>> trivialTypes;
    ctx.filter<ASTBuiltinType>([&trivialTypes, &intType, &maxLength](ASTBuiltinType* node) {
        if (!node->as<ASTVoid>()) {
            CName name;
            node->accept(name);
            trivialTypes.emplace_back(name.native, name.str);
            if (name.native.length() > maxLength) {
                maxLength = name.native.length();
            }
            if (node->as<ASTInt32>()) {
                intType = name.str;
            }
        }
    });

    beginHeader(ctx, header);
    fmt::println(header.stream, "#ifdef __cplusplus");
    fmt::println(header.stream, "# define {}_BEGIN extern \"C\" {{", API);
    fmt::println(header.stream, "# define {}_END   }}", API);
    fmt::println(header.stream, "#else");
    fmt::println(header.stream, "# define {}_BEGIN", API);
    fmt::println(header.stream, "# define {}_END", API);
    fmt::println(header.stream, "#endif");
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
    fmt::println(header.stream, "#include <stdint.h>");
    for (const auto& [native, type] : trivialTypes) {
        fmt::println(header.stream, "typedef {:<{}} {};", native, maxLength, type);
    }
    fmt::println(header.stream, "");
    constexpr auto tmpFlags = R"(#ifdef __cplusplus
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
    fmt::println(header.stream, "#define {}_TYPE({}_name) \\", API, api);
    fmt::println(header.stream, "typedef struct _##{}_name* {}_name##_t;", api, api);
    fmt::println(header.stream, "");
    ctx.filter<ASTStruct>([&header, &api](ASTStruct* node) {
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
            fmt::println(header.stream, "#define {}({}_name) \\", upper(name), api);
            fmt::println(header.stream, "typedef struct {{ \\");
            for (const auto& [key, value] : typeNames) {
                fmt::println(header.stream, "{:<{}}{:<{}} {}; \\", ' ', 4, key, maxLength, value);
            }
            fmt::println(header.stream, "}} {}_name##_h;", api);
            fmt::println(header.stream, "");
        }
    });
    endHeader(ctx, header);
}

static void generateTypes(idl::Context& ctx, const std::filesystem::path& out, bool hasInterfaces, bool hasHandles) {
    if (!hasInterfaces && !hasHandles) {
        std::filesystem::remove(out / headerStr(ctx, "types"));
        return;
    }
    auto API    = getApiPrefix(ctx, true);
    auto header = createHeader(ctx, out, "types", true);
    beginHeader(ctx, header, "platform");
    if (hasInterfaces) {
        ctx.filter<ASTInterface>([&header, &API](auto node) {
            auto name = getDeclCName(node, 2);
            fmt::println(header.stream, "{}_TYPE({})", API, name);
        });
        fmt::println(header.stream, "");
    }
    if (hasHandles) {
        ctx.filter<ASTHandle>([&header, &API](auto node) {
            auto name = getDeclCName(node, 2);
            fmt::println(header.stream, "{}_HANDLE({})", API, name);
        });
        fmt::println(header.stream, "");
    }
    endHeader(ctx, header);
}

static void generateEnums(idl::Context& ctx, const std::filesystem::path& out, bool hasEnums) {
    if (!hasEnums) {
        std::filesystem::remove(out / headerStr(ctx, "enums"));
        return;
    }
    auto API    = getApiPrefix(ctx, true);
    auto header = createHeader(ctx, out, "enums", true);
    generateDoc(ctx, header, ctx.api());
    beginHeader(ctx, header, "platform");
    ctx.filter<ASTEnum>([&ctx, &header, &API](ASTEnum* node) {
        const auto isHexOut = node->findAttr<ASTAttrHex>() != nullptr;
        std::vector<std::pair<std::string, std::string>> consts;
        consts.reserve(node->consts.size());
        size_t maxLength = 0;
        for (auto ec : node->consts) {
            consts.emplace_back(getDeclCName(ec), getDeclValue(ec, isHexOut) + ',');
            if (consts.back().first.length() > maxLength) {
                maxLength = consts.back().first.length();
            }
        }
        ASTEnumConst ec{};
        ec.parent = node;
        ec.name   = "MaxEnum";
        auto name = getDeclCName(&ec, node->findAttr<ASTAttrFlags>() ? 4 : 0);
        consts.emplace_back(name, "0x7FFFFFFF");
        if (name.length() > maxLength) {
            maxLength = name.length();
        }
        generateDoc(ctx, header, node);
        fmt::println(header.stream, "typedef enum");
        fmt::println(header.stream, "{{");
        for (const auto& [key, value] : consts) {
            fmt::println(header.stream, "{:<{}}{:<{}} = {}", ' ', 4, key, maxLength, value);
        }
        name = getDeclCName(node);
        fmt::println(header.stream, "}} {};", name);
        if (node->findAttr<ASTAttrFlags>()) {
            fmt::println(header.stream, "{}_FLAGS({})", API, name);
        }
        fmt::println(header.stream, "");
    });
    endHeader(ctx, header);
}

static void generateCallbacks(
    idl::Context& ctx, const std::filesystem::path& out, bool hasTypesHeader, bool hasEnumsHeader, bool hasCallbacks) {
    if (!hasCallbacks) {
        std::filesystem::remove(out / headerStr(ctx, "callbacks"));
        return;
    }
    auto header = createHeader(ctx, out, "callbacks", true);
    beginHeader(
        ctx, header, hasTypesHeader ? "" : "platform", hasTypesHeader ? "types" : "", hasEnumsHeader ? "enums" : "");
    ctx.filter<ASTCallback>([&header](ASTCallback* node) {
        const auto decl = fmt::format("(*{})(", getDeclCName(node));
        fmt::println(header.stream, "typedef {}", getDeclTypeCName(node));
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
    });
    endHeader(ctx, header);
}

static void generateStructs(idl::Context& ctx,
                            const std::filesystem::path& out,
                            bool hasTypesHeader,
                            bool hasEnumsHeader,
                            bool hasCallbacksHeader,
                            bool hasStructs) {
    if (!hasStructs) {
        std::filesystem::remove(out / headerStr(ctx, "structs"));
        return;
    }
    auto header = createHeader(ctx, out, "structs", true);
    beginHeader(ctx,
                header,
                hasTypesHeader ? "" : "platform",
                hasTypesHeader ? "types" : "",
                hasEnumsHeader ? "enums" : "",
                hasCallbacksHeader ? "callbacks" : "");
    ctx.filter<ASTStruct>([&header](ASTStruct* node) {
        if (!node->findAttr<ASTAttrHandle>()) {
            size_t maxLength = 0;
            std::vector<std::pair<std::string, std::string>> typeNames;
            for (auto field : node->fields) {
                typeNames.emplace_back(getTypeAndName(field));
                if (typeNames.back().first.length() > maxLength) {
                    maxLength = typeNames.back().first.length();
                }
            }
            fmt::println(header.stream, "typedef struct");
            fmt::println(header.stream, "{{");
            for (const auto& [key, value] : typeNames) {
                fmt::println(header.stream, "{:<{}}{:<{}} {};", ' ', 4, key, maxLength, value);
            }
            fmt::println(header.stream, "}} {};", getDeclCName(node));
            fmt::println(header.stream, "");
        }
    });
    endHeader(ctx, header);
}

static void generateCore(idl::Context& ctx,
                         const std::filesystem::path& out,
                         bool hasTypesHeader,
                         bool hasEnumsHeader,
                         bool hasCallbacksHeader,
                         bool hasStructsHeader) {
    auto api       = getApiPrefix(ctx, false);
    auto importApi = api + "_api";
    auto header    = createHeader(ctx, out, "core", true);
    auto printFunc = [&importApi, &header](ASTDecl* decl, const std::vector<ASTArg*>& args) {
        fmt::println(header.stream, "{} {}", importApi, getDeclTypeCName(decl));
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
    };

    generateDoc(ctx, header, ctx.api(), true);
    beginHeader(ctx,
                header,
                "version",
                (hasTypesHeader || hasStructsHeader || hasEnumsHeader) ? "" : "platform",
                hasTypesHeader ? "types" : "",
                hasEnumsHeader ? "enums" : "",
                hasCallbacksHeader ? "callbacks" : "",
                hasStructsHeader ? "structs" : "");
    ctx.filter<ASTFunc>([&header, &printFunc](auto node) {
        printFunc(node, node->args);
    });
    ctx.filter<ASTMethod>([&header, &printFunc](auto node) {
        printFunc(node, node->args);
    });
    endHeader(ctx, header);
}

void generateC(idl::Context& ctx, const std::filesystem::path& out) {
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

    generateVersion(ctx, out);
    generatePlatform(ctx, out);
    generateTypes(ctx, out, hasInterfaces, hasHandles);
    generateEnums(ctx, out, hasEnums);
    generateCallbacks(ctx, out, hasTypes, hasEnums, hasCallbacks);
    generateStructs(ctx, out, hasTypes, hasEnums, hasCallbacks, hasStructs);
    generateCore(ctx, out, hasTypes, hasEnums, hasCallbacks, hasStructs);
}
