#include "case_converter.hpp"
#include "context.hpp"

using namespace idl;

struct Package {
    int assemblyVersion;
    std::string assemblyName;
    std::string authors;
    std::string rootNamespace;
    std::string packageId;
    std::string copyright;
    std::string projectUrl;
    std::string repository;
    std::string repositoryType;
    std::string tags;
    std::string readmeFile;
    std::string licenseExpression;
    std::string licenseFile;
};

struct Stream {
    std::ostream& stream;
    std::unique_ptr<std::ofstream> fstream;
    std::unique_ptr<std::ostringstream> sstream;
    std::string filename;
    idl_write_callback_t writer;
    idl_data_t writerData;
};

struct CSharpName : Visitor {
    void visit(ASTApi* node) override {
        str = pascalCase(node);
    }

    void discarded(ASTNode*) override {
        assert(!"Default value is missing");
    }

    static std::string pascalCase(ASTDecl* decl) {
        std::vector<int>* nums = nullptr;
        if (auto attr = decl->findAttr<ASTAttrTokenizer>()) {
            nums = &attr->nums;
        }
        return convert(decl->name, Case::PascalCase, nums);
    }

    std::string str;
};

static std::string csharpName(ASTDecl* decl) {
    CSharpName name{};
    decl->accept(name);
    return name.str;
}

static std::string& ltrim(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return str;
}

static std::string& rtrim(std::string& str) {
    str.erase(std::find_if(str.rbegin(),
                           str.rend(),
                           [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(),
              str.end());
    return str;
}

static std::string& trim(std::string& str) {
    return ltrim(rtrim(str));
}

static std::pair<std::string, std::string> keyValue(const std::string& addition) {
    if (auto pos = addition.find('='); pos != std::string::npos) {
        return { addition.substr(0, pos), addition.substr(pos + 1) };
    }
    return { addition, "" };
}

static std::string docString(const std::vector<ASTNode*>& nodes) {
    std::ostringstream ss;
    for (auto node : nodes) {
        if (auto str = node->as<ASTLiteralStr>()) {
            fmt::print(ss, "{}", str->value);
        } else if (auto ref = node->as<ASTDeclRef>()) {
            assert(!"TODO");
        } else {
            assert(!"unreachable code");
        }
    }
    return ss.str();
}

static Stream createStream(idl::Context& ctx,
                           const std::filesystem::path& out,
                           const std::string& filename,
                           idl_write_callback_t writer,
                           idl_data_t writerData) {
    std::filesystem::create_directories(out);
    auto name = out / filename;
    if (writer) {
        auto stream = std::make_unique<std::ostringstream>();
        auto ptr    = stream.get();
        return { *ptr, nullptr, std::move(stream), filename, writer, writerData };
    } else {
        auto stream = std::make_unique<std::ofstream>(std::ofstream(name));
        if (stream->fail()) {
            idl::err<IDL_STATUS_E2067>(ctx.api()->location, name.string());
        }
        auto ptr = stream.get();
        return { *ptr, std::move(stream) };
    }
}

static void endStream(Stream& stream) {
    if (stream.writer) {
        const std::string data = stream.sstream->str();
        idl_source_t source{ stream.filename.c_str(), data.c_str(), (idl_uint32_t) data.length() };
        stream.writer(&source, stream.writerData);
    }
}

std::string escapeXml(const std::string& str) {
    std::ostringstream ss;
    for (auto c : str) {
        switch (c) {
            case '&':
                ss << "&amp;";
                break;
            case '<':
                ss << "&lt;";
                break;
            case '>':
                ss << "&gt;";
                break;
            case '"':
                ss << "&quot;";
                break;
            case '\'':
                ss << "&#039;";
                break;
            default:
                ss << c;
                break;
        }
    }
    return ss.str();
}

static void createSCharpProj(const Package& package,
                             idl::Context& ctx,
                             const std::filesystem::path& out,
                             idl_write_callback_t writer,
                             idl_data_t writerData) {
    constexpr auto props = R"(  <PropertyGroup>
    <TargetFrameworks>netstandard2.0;net8.0</TargetFrameworks>
    <ImplicitUsings>disable</ImplicitUsings>
    <AssemblyName>{assemblyName}</AssemblyName>
    <RootNamespace>{rootNamespace}</RootNamespace>
    <PackageId>{packageId}</PackageId>
    <Version>{version}</Version>
    <Authors>{authors}</Authors>
    <Description>{description}</Description>
    <Copyright>{copyright}</Copyright>
    <PackageProjectUrl>{projectUrl}</PackageProjectUrl>
    <RepositoryUrl>{repository}</RepositoryUrl>
    <RepositoryType>{repositoryType}</RepositoryType>
    <PackageTags>{tags}</PackageTags>
    <AssemblyVersion>{version}.{assemblyVersion}</AssemblyVersion>
    <PackageReadmeFile>{readmeFile}</PackageReadmeFile>
    {license}<AllowUnsafeBlocks>True</AllowUnsafeBlocks>
  </PropertyGroup>
)";

    auto major = 0;
    auto minor = 0;
    auto micro = 0;
    if (ctx.apiVersion()) {
        major = ctx.apiVersion().value().major;
        minor = ctx.apiVersion().value().minor;
        micro = ctx.apiVersion().value().micro;
    } else if (auto version = ctx.api()->findAttr<ASTAttrVersion>()) {
        major = version->major;
        minor = version->minor;
        micro = version->micro;
    }
    auto version = std::to_string(major) + '.' + std::to_string(minor) + '.' + std::to_string(micro);

    std::string readme;
    if (package.readmeFile.length()) {
        std::filesystem::path path(package.readmeFile);
        readme = path.filename().string();
    }

    std::string license;
    if (package.licenseExpression.length()) {
        license = "<PackageLicenseExpression>" + package.licenseExpression + "</PackageLicenseExpression>\n    ";
    } else if (package.licenseFile.length()) {
        std::filesystem::path path(package.licenseFile);
        license = "<PackageLicenseFile>" + path.filename().string() + "</PackageLicenseFile>\n    ";
    }

    std::string description;

    auto stream = createStream(ctx, out, package.assemblyName + ".csproj", writer, writerData);
    fmt::println(stream.stream, "{}", "<Project Sdk=\"Microsoft.NET.Sdk\">");
    fmt::println(stream.stream, "");
    fmt::println(stream.stream,
                 props,
                 fmt::arg("assemblyName", package.assemblyName),
                 fmt::arg("rootNamespace", package.rootNamespace),
                 fmt::arg("packageId", package.packageId),
                 fmt::arg("version", version),
                 fmt::arg("authors", escapeXml(package.authors)),
                 fmt::arg("description", description),
                 fmt::arg("copyright", escapeXml(package.copyright)),
                 fmt::arg("projectUrl", package.projectUrl),
                 fmt::arg("repository", package.repository),
                 fmt::arg("repositoryType", package.repositoryType),
                 fmt::arg("tags", escapeXml(package.tags)),
                 fmt::arg("assemblyVersion", package.assemblyVersion),
                 fmt::arg("readmeFile", readme),
                 fmt::arg("license", license));
    if (package.readmeFile.length()) {
        std::filesystem::path path(package.readmeFile);
        const auto rel = std::filesystem::relative(path, out);
        fmt::println(stream.stream,
                     R"(  <ItemGroup>
    <None Include="{}">
      <Pack>True</Pack>
      <PackagePath>\</PackagePath>
    </None>
  </ItemGroup>
)",
                     rel.string());
    }
    if (package.licenseFile.length()) {
        std::filesystem::path path(package.licenseFile);
        const auto rel = std::filesystem::relative(path, out);
        fmt::println(stream.stream,
                     R"(  <ItemGroup>
    <None Include="{}">
      <Pack>True</Pack>
      <PackagePath>\</PackagePath>
    </None>
  </ItemGroup>
)",
                     rel.string());
    }
    fmt::println(stream.stream, R"(  <ItemGroup>
    <PackageReference Include="System.Runtime.CompilerServices.Unsafe" Version="6.1.2" />
  </ItemGroup>
)");
    fmt::println(stream.stream, "</Project>");
    endStream(stream);
}

void generateCs(idl::Context& ctx,
                const std::filesystem::path& out,
                idl_write_callback_t writer,
                idl_data_t writerData,
                std::span<idl_utf8_t> additions) {
    Package package{};
    package.assemblyName = package.rootNamespace = package.packageId = csharpName(ctx.api());
    for (const auto& author : ctx.api()->doc->authors) {
        if (package.authors.length()) {
            package.authors += ',';
        }
        auto str = docString(author);
        if (auto pos = str.find('<'); pos != std::string::npos) {
            str = str.substr(0, pos);
        }
        trim(str);
        package.authors += str;
    }
    package.copyright = docString(ctx.api()->doc->license);
    for (auto arg : additions) {
        const auto [key, value] = keyValue(arg);
        if (key == "+assemblyver") {
            package.assemblyVersion = std::atoi(value.c_str());
        } else if (key == "+assemblyname") {
            package.assemblyName = value;
        } else if (key == "+authors") {
            package.authors = value;
        } else if (key == "+rootns") {
            package.rootNamespace = value;
        } else if (key == "+packageid") {
            package.packageId = value;
        } else if (key == "+copyright") {
            package.copyright = value;
        } else if (key == "+projecturl") {
            package.projectUrl = value;
        } else if (key == "+repo") {
            package.repository = value;
        } else if (key == "+repotype") {
            package.repositoryType = value;
        } else if (key == "+tags") {
            package.tags = value;
        } else if (key == "+readmefile") {
            package.readmeFile = value;
        } else if (key == "+licenseexpr") {
            package.licenseExpression = value;
        } else if (key == "+licensefile") {
            package.licenseFile = value;
        }
    }
    createSCharpProj(package, ctx, out, writer, writerData);
}
