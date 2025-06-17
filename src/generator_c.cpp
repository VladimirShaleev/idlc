#include "generator_c.hpp"
#include "case_converter.hpp"

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

static void beginHeader(idl::Context& ctx, Header& header, bool externC = true) {
    fmt::println(header.stream, "#ifndef {}", header.includeGuard);
    fmt::println(header.stream, "#define {}", header.includeGuard);
    fmt::println(header.stream, "");
    if (externC) {
        fmt::println(header.stream, "{}_BEGIN", convert(ctx.api()->name, Case::ScreamingSnakeCase));
        fmt::println(header.stream, "");
    }
}

static void endHeader(idl::Context& ctx, Header& header, bool externC = true) {
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

static void generateEnum(std::ostream& stream, ASTEnum* node) {
    const auto isHexOut = node->findAttr<ASTAttrHex>() != nullptr;
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
    if (node->findAttr<ASTAttrFlags>()) {
        name.str = name.str.substr(0, name.str.length() - 4);
    }
    consts.emplace_back(name.str, "0x7FFFFFFF");
    if (name.str.length() > maxLength) {
        maxLength = name.str.length();
    }
    node->accept(name);

    fmt::println(stream, "typedef enum");
    fmt::println(stream, "{{");
    for (const auto& [key, value] : consts) {
        fmt::println(stream, "{:<{}}{:<{}} = {}", ' ', 4, key, maxLength, value);
    }
    fmt::println(stream, "}} {};", name.str);
    fmt::println(stream, "");
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
# define {API}_FLAGS({api}_enum_t)                                                                          \
extern "C++" {{                                                                                                \
inline {API}_CONSTEXPR {api}_enum_t operator~({api}_enum_t lhr) noexcept {{                                 \
    return static_cast<{api}_enum_t>(~static_cast<{int}>(lhr));                                    \
}}                                                                                                             \
inline {API}_CONSTEXPR {api}_enum_t operator|({api}_enum_t lhr, {api}_enum_t rhs) noexcept {{              \
    return static_cast<{api}_enum_t>(static_cast<{int}>(lhr) | static_cast<{int}>(rhs)); \
}}                                                                                                             \
inline {API}_CONSTEXPR {api}_enum_t operator&({api}_enum_t lhr, {api}_enum_t rhs) noexcept {{              \
    return static_cast<{api}_enum_t>(static_cast<{int}>(lhr) & static_cast<{int}>(rhs)); \
}}                                                                                                             \
inline {API}_CONSTEXPR {api}_enum_t operator^({api}_enum_t lhr, {api}_enum_t rhs) noexcept {{              \
    return static_cast<{api}_enum_t>(static_cast<{int}>(lhr) ^ static_cast<{int}>(rhs)); \
}}                                                                                                             \
inline {API}_CONSTEXPR_14 {api}_enum_t& operator|=({api}_enum_t& lhr, {api}_enum_t rhs) noexcept {{        \
    return lhr = lhr | rhs;                                                                                   \
}}                                                                                                             \
inline {API}_CONSTEXPR_14 {api}_enum_t& operator&=({api}_enum_t& lhr, {api}_enum_t rhs) noexcept {{        \
    return lhr = lhr & rhs;                                                                                   \
}}                                                                                                             \
inline {API}_CONSTEXPR_14 {api}_enum_t& operator^=({api}_enum_t& lhr, {api}_enum_t rhs) noexcept {{        \
    return lhr = lhr ^ rhs;                                                                                   \
}}                                                                                                             \
}}
#else
# define {API}_FLAGS(gerium_enum_t)
#endif)",
                 fmt::arg("API", API),
                 fmt::arg("api", api),
                 fmt::arg("int", intType));
    fmt::println(header.stream, "");

    endHeader(ctx, header, false);
}

static void generateEnums(idl::Context& ctx, const std::filesystem::path& out) {
    auto header = createHeader(ctx, out, "enums");
    beginHeader(ctx, header);
    ctx.filter<ASTEnum>([&header](auto node) {
        generateEnum(header.stream, node);
    });
    endHeader(ctx, header);
}

void GeneratorC::generate(idl::Context& ctx, const std::filesystem::path& out) {
    generatePlatform(ctx, out);
    generateEnums(ctx, out);
}
