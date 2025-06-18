
#include "case_converter.hpp"
#include "context.hpp"

#include <fmt/ostream.h>
#include <fstream>

struct Header {
    std::ofstream stream;
    std::string filename;
    std::string includeGuard;

    friend std::ostream& operator<<(std::ostream& os, const Header& header) {
        return os << header.filename;
    }
};

static std::string headerStr(idl::Context& ctx, std::string postfix = "") {
    return convert(ctx.api()->name, Case::LispCase) + (postfix.length() ? "-" + lower(postfix) : "") + ".h";
}

static std::string includeGuardStr(idl::Context& ctx, std::string postfix = "") {
    return convert(ctx.api()->name, Case::ScreamingSnakeCase) + (postfix.length() ? "_" + upper(postfix) : "") + "_H";
}

static Header createHeader(idl::Context& ctx, const std::filesystem::path& out, const std::string& postfix) {
    auto header = out / headerStr(ctx, postfix);
    auto guard  = includeGuardStr(ctx, postfix);
    auto stream = std::ofstream(header);
    if (stream.fail()) {
        std::cerr << fmt::format("failed to create file '{}'", header.string()) << std::endl;
        exit(EXIT_FAILURE);
    }
    return { std::move(stream), header.filename().string(), guard };
}

static void writeDoc(Header& header, ASTDoc* doc, bool align) {
}

static std::pair<std::string, std::string> fieldTypeName(ASTField* field) {
    auto attrType  = field->template findAttr<ASTAttrType>();
    auto attrArray = field->template findAttr<ASTAttrArray>();
    auto attrConst = field->template findAttr<ASTAttrConst>() != nullptr;
    auto attrRef   = field->template findAttr<ASTAttrRef>() != nullptr;
    CName name;
    attrType->type->decl->accept(name);
    auto type = attrConst ? "const " + name.str : name.str;
    if (attrRef) {
        type += '*';
    }
    field->accept(name);
    auto fieldName = name.str;
    if (attrArray && !attrArray->ref) {
        fieldName += attrArray->size > 1 ? '[' + std::to_string(attrArray->size) + ']' : "";
    }
    return { type, fieldName };
}

static std::pair<std::string, std::string> argTypeName(ASTArg* arg) {
    CName name;
    arg->findAttr<ASTAttrType>()->type->decl->accept(name);
    auto typeStr = name.str;
    if (arg->findAttr<ASTAttrConst>()) {
        typeStr = "const " + typeStr;
    }
    if (arg->findAttr<ASTAttrOut>()) {
        typeStr += '*';
    }
    arg->accept(name);
    const auto nameStr = name.str;
    return { typeStr, name.str };
}

template <typename... Includes>
static void beginHeader(idl::Context& ctx, Header& header, bool externC, const Includes&... includes) {
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
    if (externC) {
        fmt::println(header.stream, "{}_BEGIN", convert(ctx.api()->name, Case::ScreamingSnakeCase));
        fmt::println(header.stream, "");
    }
}

static void endHeader(idl::Context& ctx, Header& header, bool externC) {
    if (externC) {
        fmt::println(header.stream, "{}_END", convert(ctx.api()->name, Case::ScreamingSnakeCase));
        fmt::println(header.stream, "");
    }
    fmt::println(header.stream, "#endif /* {} */", header.includeGuard);
}

static std::pair<std::string, std::string> getEnumConstKeyValue(ASTEnumConst* ec, bool isHexOut) {
    std::string value;
    CName name{};
    auto literal = ec->findAttr<ASTAttrValue>()->value;
    if (auto integer = literal->as<ASTLiteralInt>()) {
        if (isHexOut) {
            auto num         = (int32_t) integer->value;
            size_t hexLength = (num == 0) ? 1 : static_cast<size_t>(std::log2(num) / 4 + 1);
            size_t width     = (hexLength % 2 == 0) ? hexLength : hexLength + 1;
            value            = fmt::format("0x{:0{}x}", num, width);
        } else {
            value = fmt::format("{}", (int32_t) integer->value);
        }
    } else if (auto refs = literal->as<ASTLiteralConsts>()) {
        for (auto ref : refs->decls) {
            ref->decl->accept(name);
            value += value.length() > 0 ? " | " + name.str : name.str;
        }
    } else {
        assert(!"unreachable code");
    }
    ec->accept(name);
    return { name.str, value };
}

static void generateVersion(idl::Context& ctx, const std::filesystem::path& out) {
    auto API           = convert(ctx.api()->name, Case::ScreamingSnakeCase);
    auto header        = createHeader(ctx, out, "version");
    constexpr auto tmp = R"(#define {API}_VERSION_MAJOR 0
#define {API}_VERSION_MINOR 0
#define {API}_VERSION_MICRO 0

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

    beginHeader(ctx, header, false);
    fmt::println(header.stream, tmp, fmt::arg("API", API));
    endHeader(ctx, header, false);
}

static void generatePlatform(idl::Context& ctx, const std::filesystem::path& out) {
    auto API       = convert(ctx.api()->name, Case::ScreamingSnakeCase);
    auto api       = convert(ctx.api()->name, Case::SnakeCase);
    auto dllPublic = api + "_api";
    auto header    = createHeader(ctx, out, "platform");
    auto intType   = std::string();

    size_t maxNativeTypeLength = 0;
    std::vector<std::pair<std::string, std::string>> trivialTypes;
    ctx.filter<ASTBuiltinType>([&trivialTypes, &intType, &maxNativeTypeLength](auto node) {
        if (!node->template as<ASTVoid>()) {
            CName name;
            node->accept(name);
            trivialTypes.emplace_back(name.native, name.str);
            if (name.native.length() > maxNativeTypeLength) {
                maxNativeTypeLength = name.native.length();
            }
            if (node->template as<ASTInt32>()) {
                intType = name.str;
            }
        }
    });

    beginHeader(ctx, header, false);
    fmt::println(header.stream, "#ifdef __cplusplus");
    fmt::println(header.stream, "# define {}_BEGIN extern \"C\" {{", API);
    fmt::println(header.stream, "# define {}_END   }}", API);
    fmt::println(header.stream, "#else");
    fmt::println(header.stream, "# define {}_BEGIN", API);
    fmt::println(header.stream, "# define {}_END", API);
    fmt::println(header.stream, "#endif");
    fmt::println(header.stream, "");
    fmt::println(header.stream, "#ifndef {}", dllPublic);
    fmt::println(header.stream, "# if defined(_MSC_VER) && !defined({}_STATIC_BUILD)", API);
    fmt::println(header.stream, "#  define {} __declspec(dllimport)", dllPublic);
    fmt::println(header.stream, "# else");
    fmt::println(header.stream, "#  define {}", dllPublic);
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
        fmt::println(header.stream, "typedef {:<{}} {};", native, maxNativeTypeLength, type);
    }
    fmt::println(header.stream, "");
    fmt::println(header.stream,
                 R"(#ifdef __cplusplus
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
#endif)",
                 fmt::arg("API", API),
                 fmt::arg("api", api),
                 fmt::arg("int", intType));
    fmt::println(header.stream, "");
    fmt::println(header.stream, "#define {}_TYPE({}_name) \\", API, api);
    fmt::println(header.stream, "typedef struct _##{}_name* {}_name##_t;", api, api);
    fmt::println(header.stream, "");
    ctx.filter<ASTStruct>([&header, &api](auto node) {
        if (node->template findAttr<ASTAttrHandle>()) {
            size_t maxLength = 0;
            std::vector<std::pair<std::string, std::string>> typeNames;
            for (auto field : node->fields) {
                typeNames.emplace_back(fieldTypeName(field));
                if (typeNames.back().first.length() > maxLength) {
                    maxLength = typeNames.back().first.length();
                }
            }
            CName name;
            node->accept(name);
            name.str = upper(name.str).substr(0, name.str.length() - 2);
            fmt::println(header.stream, "#define {}({}_name) \\", name.str, api);
            fmt::println(header.stream, "typedef struct {{ \\");
            for (const auto& [key, value] : typeNames) {
                fmt::println(header.stream, "{:<{}}{:<{}} {}; \\", ' ', 4, key, maxLength, value);
            }
            fmt::println(header.stream, "}} {}_name##_h;", api);
            fmt::println(header.stream, "");
        }
    });
    endHeader(ctx, header, false);
}

static void generateTypes(idl::Context& ctx, const std::filesystem::path& out, bool hasInterfaces, bool hasHandles) {
    if (!hasInterfaces && !hasHandles) {
        std::filesystem::remove(out / headerStr(ctx, "types"));
        return;
    }

    auto API    = convert(ctx.api()->name, Case::ScreamingSnakeCase);
    auto header = createHeader(ctx, out, "types");

    beginHeader(ctx, header, true, "platform");
    if (hasInterfaces) {
        ctx.filter<ASTInterface>([&header, &API](auto node) {
            CName name;
            node->accept(name);
            name.str = name.str.substr(0, name.str.length() - 2);
            fmt::println(header.stream, "{}_TYPE({})", API, name.str);
        });
        fmt::println(header.stream, "");
    }
    if (hasHandles) {
        ctx.filter<ASTHandle>([&header, &API](auto node) {
            CName name;
            node->accept(name);
            name.str = name.str.substr(0, name.str.length() - 2);
            fmt::println(header.stream, "{}_HANDLE({})", API, name.str);
        });
        fmt::println(header.stream, "");
    }
    endHeader(ctx, header, true);
}

static void generateEnums(idl::Context& ctx, const std::filesystem::path& out, bool hasEnums) {
    if (!hasEnums) {
        std::filesystem::remove(out / headerStr(ctx, "enums"));
        return;
    }

    auto API    = convert(ctx.api()->name, Case::ScreamingSnakeCase);
    auto header = createHeader(ctx, out, "enums");
    beginHeader(ctx, header, true, "platform");
    ctx.filter<ASTEnum>([&header, &API](auto node) {
        const auto isHexOut = node->template findAttr<ASTAttrHex>() != nullptr;
        std::vector<std::pair<std::string, std::string>> consts;
        consts.reserve(node->consts.size());
        size_t maxLength = 0;
        for (auto ec : node->consts) {
            const auto [key, value] = getEnumConstKeyValue(ec, isHexOut);
            consts.emplace_back(key, value + ',');
            if (consts.back().first.length() > maxLength) {
                maxLength = consts.back().first.length();
            }
        }
        CName name{};
        ASTEnumConst ec{};
        ec.parent = node;
        ec.name   = "MaxEnum";
        ec.accept(name);
        if (node->template findAttr<ASTAttrFlags>()) {
            name.str = name.str.substr(0, name.str.length() - 4);
        }
        consts.emplace_back(name.str, "0x7FFFFFFF");
        if (name.str.length() > maxLength) {
            maxLength = name.str.length();
        }
        node->accept(name);

        fmt::println(header.stream, "typedef enum");
        fmt::println(header.stream, "{{");
        for (const auto& [key, value] : consts) {
            fmt::println(header.stream, "{:<{}}{:<{}} = {}", ' ', 4, key, maxLength, value);
        }
        fmt::println(header.stream, "}} {};", name.str);
        if (node->template findAttr<ASTAttrFlags>()) {
            fmt::println(header.stream, "{}_FLAGS({})", API, name.str);
        }
        fmt::println(header.stream, "");
    });
    endHeader(ctx, header, true);
}

static void generateCallbacks(
    idl::Context& ctx, const std::filesystem::path& out, bool hasTypesHeader, bool hasEnumsHeader, bool hasCallbacks) {
    if (!hasCallbacks) {
        std::filesystem::remove(out / headerStr(ctx, "callbacks"));
        return;
    }
    auto header = createHeader(ctx, out, "callbacks");
    beginHeader(ctx,
                header,
                true,
                hasTypesHeader ? "" : "platform",
                hasTypesHeader ? "types" : "",
                hasEnumsHeader ? "enums" : "");
    ctx.filter<ASTCallback>([&header](auto node) {
        CName name;
        node->template findAttr<ASTAttrType>()->type->decl->accept(name);
        const auto returnType = name.str;
        node->accept(name);
        const auto callbackName = name.str;
        const auto decl         = fmt::format("(*{})(", callbackName);
        fmt::println(header.stream, "typedef {}", returnType);
        fmt::print(header.stream, "{}", decl);
        if (node->args.empty()) {
            fmt::print(header.stream, "void");
        } else {
            for (size_t i = 0; i < node->args.size(); ++i) {
                const auto [argType, argName] = argTypeName(node->args[i]);
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
    endHeader(ctx, header, true);
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

    auto header = createHeader(ctx, out, "structs");
    beginHeader(ctx,
                header,
                true,
                hasTypesHeader ? "" : "platform",
                hasTypesHeader ? "types" : "",
                hasEnumsHeader ? "enums" : "",
                hasCallbacksHeader ? "callbacks" : "");
    ctx.filter<ASTStruct>([&header](auto node) {
        if (!node->template findAttr<ASTAttrHandle>()) {
            size_t maxLength = 0;
            std::vector<std::pair<std::string, std::string>> typeNames;
            for (auto field : node->fields) {
                typeNames.emplace_back(fieldTypeName(field));
                if (typeNames.back().first.length() > maxLength) {
                    maxLength = typeNames.back().first.length();
                }
            }
            CName name{};
            node->accept(name);
            fmt::println(header.stream, "typedef struct");
            fmt::println(header.stream, "{{");
            for (const auto& [key, value] : typeNames) {
                fmt::println(header.stream, "{:<{}}{:<{}} {};", ' ', 4, key, maxLength, value);
            }
            fmt::println(header.stream, "}} {};", name.str);
            fmt::println(header.stream, "");
        }
    });
    endHeader(ctx, header, true);
}

static void generateCore(idl::Context& ctx,
                         const std::filesystem::path& out,
                         bool hasTypesHeader,
                         bool hasEnumsHeader,
                         bool hasCallbacksHeader,
                         bool hasStructsHeader) {
    auto api       = convert(ctx.api()->name, Case::SnakeCase);
    auto dllPublic = api + "_api";
    auto header    = createHeader(ctx, out, "core");
    auto printFunc = [&dllPublic, &header](ASTDecl* decl, const std::vector<ASTArg*>& args) {
        CName name;
        decl->template findAttr<ASTAttrType>()->type->decl->accept(name);
        fmt::println(header.stream, "{} {}", dllPublic, name.str);

        decl->accept(name);
        auto declStr = fmt::format("{}(", name.str);
        fmt::print(header.stream, "{}", declStr);

        if (args.empty()) {
            fmt::print(header.stream, "void");
        } else {
            for (size_t i = 0; i < args.size(); ++i) {
                const auto [typeStr, nameStr] = argTypeName(args[i]);
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

    beginHeader(ctx,
                header,
                true,
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
    endHeader(ctx, header, true);
}

void generateC(idl::Context& ctx, const std::filesystem::path& out) {
    auto hasInterfaces = !ctx.filter<ASTInterface>([](auto) {
        return false;
    });
    auto hasHandles    = !ctx.filter<ASTHandle>([](auto) {
        return false;
    });
    auto hasEnums      = !ctx.filter<ASTEnum>([](auto) {
        return false;
    });
    auto hasCallbacks  = !ctx.filter<ASTCallback>([](auto) {
        return false;
    });
    bool hasStructs    = false;
    ctx.filter<ASTStruct>([&hasStructs](auto node) {
        if (!node->template findAttr<ASTAttrHandle>()) {
            hasStructs = true;
            return false;
        }
        return true;
    });
    auto hasTypes = hasInterfaces || hasHandles;

    generateVersion(ctx, out);
    generatePlatform(ctx, out);
    generateTypes(ctx, out, hasInterfaces, hasHandles);
    generateEnums(ctx, out, hasEnums);
    generateCallbacks(ctx, out, hasTypes, hasEnums, hasCallbacks);
    generateStructs(ctx, out, hasTypes, hasEnums, hasCallbacks, hasStructs);
    generateCore(ctx, out, hasTypes, hasEnums, hasCallbacks, hasStructs);
}
