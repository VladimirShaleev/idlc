#include "case_converter.hpp"
#include "context.hpp"

#include <stduuid/uuid.h>

using namespace idl;

struct Package {
    std::string dllosx;
    std::string dllwin32;
    std::string dllwin64;
    std::string dlllinux;
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
        str = changeCase(node);
    }

    void visit(ASTEnum* node) override {
        str = changeCase(node);
    }

    void visit(ASTEnumConst* node) override {
        str = changeCase(node);
    }

    void discarded(ASTNode*) override {
        assert(!"Default value is missing");
    }

    static std::string changeCase(ASTDecl* decl, Case newCase = Case::PascalCase) {
        std::vector<int>* nums = nullptr;
        if (auto attr = decl->findAttr<ASTAttrTokenizer>()) {
            nums = &attr->nums;
        }
        return convert(decl->name, newCase, nums);
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

static void createTargets(const Package& package,
                          idl::Context& ctx,
                          const std::filesystem::path& out,
                          idl_write_callback_t writer,
                          idl_data_t writerData) {
    const auto apiName = csharpName(ctx.api());

    auto getName = [&apiName](const std::string& fullname, const std::string& prefix, const std::string& ext) {
        return fullname.length() ? std::filesystem::path(fullname).filename().string() : prefix + apiName + ext;
    };

    const auto winName   = getName(package.dllwin64, "", ".dll");
    const auto osxName   = getName(package.dllosx, "lib", ".dylib");
    const auto linuxName = getName(package.dlllinux, "lib", ".so");

    constexpr auto targets = R"xml(<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <_IsWindows Condition="'$([System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform($([System.Runtime.InteropServices.OSPlatform]::Windows)))' == 'true'">true</_IsWindows>
    <_IsMacOS Condition="'$([System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform($([System.Runtime.InteropServices.OSPlatform]::OSX)))' == 'true'">true</_IsMacOS>
    <_IsLinux Condition="'$([System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform($([System.Runtime.InteropServices.OSPlatform]::Linux)))' == 'true'">true</_IsLinux>

    <_NativeRuntime Condition=" '$(_NativeRuntime)' == '' And '$(_IsMacOS)' == 'true' And '$(PlatformTarget)' == 'x64'">osx</_NativeRuntime>
    <_NativeRuntime Condition=" '$(_NativeRuntime)' == '' And '$(_IsMacOS)' == 'true' And '$(PlatformTarget)' == 'ARM64'">osx</_NativeRuntime>
    <_NativeRuntime Condition=" '$(_NativeRuntime)' == '' And '$(_IsLinux)' == 'true' And ('$(Prefer32Bit)' == 'false' Or '$(PlatformTarget)' == 'x64')">linux-x64</_NativeRuntime>
    <_NativeRuntime Condition=" '$(_NativeRuntime)' == '' And '$(_IsWindows)' == 'true' And ('$(Prefer32Bit)' == 'true' Or '$(PlatformTarget)' == 'x86')">win-x86</_NativeRuntime>
    <_NativeRuntime Condition=" '$(_NativeRuntime)' == '' And '$(_IsWindows)' == 'true' And ('$(Prefer32Bit)' == 'false' Or '$(PlatformTarget)' == 'x64')">win-x64</_NativeRuntime>

    <_NativeLibName Condition="'$(_NativeRuntime)' == 'win-x86' Or '$(_NativeRuntime)' == 'win-x64'">{}</_NativeLibName>
    <_NativeLibName Condition="'$(_NativeRuntime)' == 'osx'">{}</_NativeLibName>
    <_NativeLibName Condition="'$(_NativeRuntime)' == 'linux-x64'">{}</_NativeLibName>
  </PropertyGroup>
  <ItemGroup>
    <Content Condition="'$(_NativeRuntime)' != ''" Include="$(MSBuildThisFileDirectory)..\..\runtimes\$(_NativeRuntime)\native\$(_NativeLibName)">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
      <Visible>False</Visible>
    </Content>
  </ItemGroup>
</Project>
)xml";

    auto stream = createStream(ctx, out, package.assemblyName + ".targets", writer, writerData);
    fmt::println(stream.stream, targets, winName, osxName, linuxName);
    endStream(stream);
}

static void createProj(const Package& package,
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
    <GenerateDocumentationFile>True</GenerateDocumentationFile>
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

    fmt::println(stream.stream, "  <ItemGroup>");
    auto addDll = [&stream, &out](const std::string& fullpath, const std::string& folder) {
        if (fullpath.length()) {
            std::filesystem::path path(fullpath);
            const auto rel = std::filesystem::relative(path, out).string();
            fmt::println(stream.stream,
                         R"(    <Content Include="{}">
      <PackagePath>runtimes/{}/native</PackagePath>
      <Pack>true</Pack>
    </Content>)",
                         rel,
                         folder);
        }
    };
    addDll(package.dllwin32, "win-x86");
    addDll(package.dllwin64, "win-x64");
    const auto targets = package.assemblyName + ".targets";
    fmt::println(stream.stream,
                 R"(    <Content Include="{targets}">
      <PackagePath>build/net40/{targets}</PackagePath>
      <Pack>true</Pack>
    </Content>)",
                 fmt::arg("targets", targets));
    fmt::println(stream.stream, "  </ItemGroup>");
    fmt::println(stream.stream, "");

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

static void createSln(const Package& package,
                      idl::Context& ctx,
                      const std::filesystem::path& out,
                      idl_write_callback_t writer,
                      idl_data_t writerData) {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{ generator };

    const auto solutionGuid = uuids::to_string(gen());
    const auto projectGuid  = uuids::to_string(gen());

    constexpr auto sln = R"(
Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 17
VisualStudioVersion = 17.13.35931.197
MinimumVisualStudioVersion = 10.0.40219.1
Project("{{{solution}}}") = "{assembly}", "{assembly}.csproj", "{{{project}}}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Release|Any CPU = Release|Any CPU
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{{{project}}}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		{{{project}}}.Debug|Any CPU.Build.0 = Debug|Any CPU
		{{{project}}}.Release|Any CPU.ActiveCfg = Release|Any CPU
		{{{project}}}.Release|Any CPU.Build.0 = Release|Any CPU
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {{A81396C2-22AA-4DF6-B7CA-48278CA8DB61}}
	EndGlobalSection
EndGlobal)";

    auto stream = createStream(ctx, out, package.assemblyName + ".sln", writer, writerData);
    fmt::println(stream.stream,
                 sln,
                 fmt::arg("assembly", package.assemblyName),
                 fmt::arg("solution", solutionGuid),
                 fmt::arg("project", projectGuid));
    endStream(stream);
}

static void createDocComment(std::ostream& stream, int indent) {
    fmt::print(stream, "{:<{}}/// ", ' ', indent);
}

static void createDocField(std::ostream& stream, int indent, const std::vector<ASTNode*> nodes) {
    std::istringstream doc(docString(nodes));
    std::string line;
    while (std::getline(doc, line, '\n')) {
        createDocComment(stream, indent);
        fmt::println(stream, "{}", line);
    }
}

static void createDoc(std::ostream& stream, int indent, const ASTDoc* doc) {
    if (!doc->brief.empty() || !doc->detail.empty()) {
        createDocComment(stream, indent);
        fmt::println(stream, "<summary>");
        if (!doc->brief.empty()) {
            createDocField(stream, indent, doc->brief);
        }
        if (!doc->detail.empty()) {
            createDocField(stream, indent, doc->detail);
        }
        createDocComment(stream, indent);
        fmt::println(stream, "</summary>");
    }
}

static void createEnums(const Package& package,
                        idl::Context& ctx,
                        const std::filesystem::path& out,
                        idl_write_callback_t writer,
                        idl_data_t writerData) {
    auto stream = createStream(ctx, out, "Enums.cs", writer, writerData);
    fmt::println(stream.stream, "using System;");
    fmt::println(stream.stream, "");
    fmt::println(stream.stream, "namespace {}", package.rootNamespace);
    fmt::println(stream.stream, "{{");
    auto first = true;
    ctx.filter<ASTEnum>([&stream, &first](ASTEnum* node) {
        if (!first) {
            fmt::println(stream.stream, "");
        }
        first = false;
        createDoc(stream.stream, 4, node->doc);
        if (node->findAttr<ASTAttrFlags>()) {
            fmt::println(stream.stream, "    [Flags]");
        }
        fmt::println(stream.stream, "    public enum {}", csharpName(node));
        fmt::println(stream.stream, "    {{");
        for (size_t i = 0; i < node->consts.size(); ++i) {
            const auto isLast = i + 1 == node->consts.size();

            auto ec      = node->consts[i];
            auto literal = ec->findAttr<ASTAttrValue>()->value;
            std::string value;
            if (auto litConsts = literal->as<ASTLiteralConsts>(); litConsts && node->findAttr<ASTAttrFlags>()) {
                for (auto decl : litConsts->decls) {
                    if (value.length()) {
                        value += " | ";
                    }
                    value += csharpName(decl->decl);
                }
            } else {
                value = std::to_string(ec->value);
            }
            fmt::println(stream.stream, "");
            createDoc(stream.stream, 8, ec->doc);
            fmt::println(stream.stream, "        {} = {}{}", csharpName(ec), value, isLast ? "" : ",");
        }
        fmt::println(stream.stream, "    }}");
    });
    fmt::println(stream.stream, "}}");
    endStream(stream);
}

static void createNative(const Package& package,
                         idl::Context& ctx,
                         const std::filesystem::path& out,
                         idl_write_callback_t writer,
                         idl_data_t writerData) {
    std::vector<ASTEnum*> checkEnums;
    ctx.filter<ASTEnum>([&checkEnums](ASTEnum* node) {
        if (node->findAttr<ASTAttrErrorCode>()) {
            checkEnums.push_back(node);
        }
    });

    auto stream = createStream(ctx, out, "NativeWrapper.cs", writer, writerData);
    fmt::println(stream.stream, "using System;");
    fmt::println(stream.stream, "using System.Collections.Generic;");
    fmt::println(stream.stream, "using System.Runtime.InteropServices;");
    fmt::println(stream.stream, "");
    fmt::println(stream.stream, "namespace {}", package.rootNamespace);
    fmt::println(stream.stream, "{{");
    fmt::println(stream.stream, "    internal unsafe static class NativeWrapper");
    fmt::println(stream.stream, "    {{");
    for (auto en : checkEnums) {
        fmt::println(stream.stream, "        public static void Check({} result)", csharpName(en));
        fmt::println(stream.stream, "        {{");
        fmt::println(stream.stream, "        }}");
    }
    fmt::println(stream.stream, "    }}");
    fmt::println(stream.stream, "}}");
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
        if (key == "+dllosx") {
            package.dllosx = value;
        } else if (key == "+dllwin32") {
            package.dllwin32 = value;
        } else if (key == "+dllwin64") {
            package.dllwin64 = value;
        } else if (key == "+dlllinux") {
            package.dlllinux = value;
        } else if (key == "+assemblyver") {
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
    createTargets(package, ctx, out, writer, writerData);
    createProj(package, ctx, out, writer, writerData);
    createSln(package, ctx, out, writer, writerData);
    createEnums(package, ctx, out, writer, writerData);
    createNative(package, ctx, out, writer, writerData);
}
