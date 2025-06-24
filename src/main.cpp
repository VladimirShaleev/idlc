#include <argparse/argparse.hpp>
#include <regex>

#include "parser.hpp"
#include "scanner.hpp"
#include <idlc/idl.h>

enum struct GeneratorType {
    C,
    Cpp
};

void generateC(idl::Context& ctx, const std::filesystem::path& out);

void addGeneratorArg(argparse::ArgumentParser& program) {
    auto& arg = program.add_argument("-g", "--generator");
    std::ostringstream help;
    help << "generator programming language (";
    bool first = true;
    for (auto& gen : magic_enum::enum_names<GeneratorType>()) {
        auto str = std::string(gen.data());
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        arg.add_choice(str);
        if (!first) {
            help << ", ";
        }
        first = false;
        help << str;
    }
    help << ')';
    arg.help(help.str());
}

GeneratorType getGeneratorArg(argparse::ArgumentParser& program) {
    if (!program.is_used("--generator")) {
        return GeneratorType::C;
    }
    auto str = program.get("--generator");
    return magic_enum::enum_cast<GeneratorType>(str, magic_enum::case_insensitive).value();
}

constexpr char s[] = R"(
@ IDLC    
@ ```
    IDLC - Interface Definition Language Compiler. It is an abstract API 
    description language for platform- and language-independent interaction 
    with the implemented interface.``` [detail]
@ Vladimir Shaleev <vladimirshaleev@gmail.com> [author]
@ MIT License [copyright]
@ ```
    MIT License

    Copyright (c) 2025 Vladimir Shaleev

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Contributor(s):
       Vladimir Shaleev <vladimirshaleev@gmail.com>``` [license]
api Idl [version(0,0,0)]

@ Warning/Error codes.
@ Here are the warning and error codes that may occur during compilation. [detail]
import Results

@ TODO.
@ TODO. [detail]
import Options

@ Generation language
@ Enumeration possible languages for generating interfaces and wrapping C libraries for other languages. [note]
enum Generator
    const C @ C generator
    // const Cpp @ TODO:
    // const Python @ TODO:
    // const Js @ TODO:
    // const TypeScript @ TODO:
    // const CSharp @ TODO:
    // const Java @ TODO:

@ Current library version as packed 32-bit value.
@ Format: (major << 16) | (minor << 8) | micro. [detail]
@ Return packed version number [return]
func Version {Uint32}

@ Current library version as human-readable string.
@ Format: "major.minor.micro", eg: "{Major}.{Minor}.{Micro}". [detail]
@ Return version string. [return]
func VersionString {Str}

@ Compiler interface.
@ Interface for interacting with the compiler. [detail]
interface Compiler
    @ Creates new compiler instance.
    @ TODO: [detail]
    @ TODO: [return]
    method Create {Result} [ctor]
        arg Compiler {Compiler} [result] @ New compiler instance.

    @ Increments reference count.
    @ Manages compiler instance lifetime. [detail]
    @ Reference to same compiler. [return]
    @ {Destroy} [see]
    method Reference {Compiler} [ref]
        arg Compiler {Compiler} [this] @ Target compiler instance.

    @ Releases compiler instance.
    @ Destroys when reference count reaches zero. [detail]
    @ {Reference} [see]
    method Destroy [destroy]
        arg Compiler {Compiler} [this] @ Compiler to destroy.

    @ Comile IDL.
    @ Compilation result. [return]
    method Compile {Result}
        arg Compiler {Compiler} [this] @ Target compiler.
        arg Generator {Generator} @ Target of generator.
        arg File {Str} @ Path to .idl file for compile.
        arg SourceCount {Uint32} @ TODO
        arg Sources {Source} [const,array(SourceCount)] @ TODO
        arg Options {Options} @ Compile options, may be null.
        arg Result {CompilationResult} [result] @ TODO.

)";

constexpr char s2[] = R"(
@ Source code.
@ Description of the source code. [detail]
struct Source
    field Name {Str} @ Name
    field Data {Str} @ The source code is passed in case of reading from memory, otherwise set to null.
    field Size {Uint32} @ Size of {Data} in bytes.

@ TODO.
@ TODO. [detail]
callback ImportCallback {Bool}
    arg Name {Str} @ TODO.
    arg Depth {Uint32} @ TODO.
    arg Data {Data} [userdata] @ TODO.
    arg Source {Source} [result] @ TODO.

@ TODO.
@ TODO. [detail]
callback ReleaseImportCallback
    arg Source {Source} [ref] @ TODO.
    arg Data {Data} [userdata] @ TODO.

@ TODO.
@ TODO. [detail]
callback WriteCallback
    arg Source {Source} [const,ref] @ TODO.
    arg Data {Data} [userdata] @ TODO.

@ TODO.
@ TODO. [detail]
interface Options
    prop DebugMode [get(GetDebugMode),set(SetDebugMode)] @ Enable debug mode.
    prop WarningsAsErrors [get(GetWarningsAsErrors),set(SetWarningsAsErrors)] @ TODO.
    prop OutputDir [get(GetOutputDir),set(SetOutputDir)] @ TODO.
    prop ImportDirs [get(GetImportDirs),set(SetImportDirs)] @ TODO.

    event Importer [get(GetImporter),set(SetImporter)] @ TODO.
    event ReleaseImport [get(GetReleaseImport),set(SetReleaseImport)] @ TODO.
    event Writer [get(GetWriter),set(SetWriter)] @ TODO.

    @ Creates new options instance.
    @ TODO: [detail]
    @ TODO: [return]
    method Create {Result} [ctor]
        arg Options {Options} [result] @ New options instance.

    @ Increments reference count.
    @ Manages options instance lifetime. [detail]
    @ Reference to same options. [return]
    @ {Destroy} [see]
    method Reference {Options} [ref]
        arg Options {Options} [this] @ Target options instance.

    @ Releases options instance.
    @ Destroys when reference count reaches zero. [detail]
    @ {Reference} [see]
    method Destroy [destroy]
        arg Options {Options} [this] @ Options to destroy.

    @ Get debug mode.
    @ Return *{True}* is debug mode enabled. [detail]
    @ *{True}* is enabled. [return]
    @ {SetDebugMode} [see]
    method GetDebugMode {Bool} [const]
        arg Options {Options} [this] @ Target options.

    @ Set debug mode.
    @ Enable debug mode. [detail]
    @ {GetDebugMode} [see]
    method SetDebugMode
        arg Options {Options} [this] @ Target options.
        arg Enable {Bool} @ Enable debug.

    @ TODO.
    @ TODO. [detail]
    @ TODO. [return]
    @ {SetWarningsAsErrors} [see]
    method GetWarningsAsErrors {Bool} [const]
        arg Options {Options} [this] @ Target options.

    @ TODO.
    @ TODO. [detail]
    @ {GetWarningsAsErrors} [see]
    method SetWarningsAsErrors
        arg Options {Options} [this] @ Target options.
        arg Enable {Bool} @ TODO.

    @ TODO.
    @ TODO. [detail]
    @ TODO. [return]
    @ {SetOutputDir} [see]
    method GetOutputDir {Str} [const]
        arg Options {Options} [this] @ Target options.

    @ TODO.
    @ TODO. [detail]
    @ {GetOutputDir} [see]
    method SetOutputDir
        arg Options {Options} [this] @ Target options.
        arg Dir {Str} @ TODO.

    @ TODO.
    @ TODO. [detail]
    @ TODO. [return]
    @ {SetImportDirs} [see]
    method GetImportDirs [const]
        arg Options {Options} [this] @ Target options.
        arg DirCount {Uint32} [in,out] @ Number of directories.
        arg Dirs {Str} [result,array(DirCount)] @ Import directories.

    @ TODO.
    @ TODO. [detail]
    @ {GetImportDirs} [see]
    method SetImportDirs
        arg Options {Options} [this] @ Target options.
        arg DirCount {Uint32} @ Number of directories.
        arg Dirs {Str} [const,array(DirCount)] @ Import directories.

    @ TODO.
    @ TODO. [detail]
    @ TODO. [return]
    method GetImporter {ImportCallback} [const]
        arg Options {Options} [this] @ Target options.
        arg Data {Data} [out,userdata] @ Callback user data pointer.

    @ TODO.
    @ TODO. [detail]
    method SetImporter
        arg Options {Options} [this] @ Target options.
        arg Callback {ImportCallback} @ Callback function.
        arg Data {Data} [userdata] @ Callback user data.

    @ TODO.
    @ TODO. [detail]
    @ TODO. [return]
    method GetReleaseImport {ReleaseImportCallback} [const]
        arg Options {Options} [this] @ Target options.
        arg Data {Data} [out,userdata] @ Callback user data pointer.

    @ TODO.
    @ TODO. [detail]
    method SetReleaseImport
        arg Options {Options} [this] @ Target options.
        arg Callback {ReleaseImportCallback} @ Callback function.
        arg Data {Data} [userdata] @ Callback user data.

    @ TODO.
    @ TODO. [detail]
    @ TODO. [return]
    method GetWriter {WriteCallback} [const]
        arg Options {Options} [this] @ Target options.
        arg Data {Data} [out,userdata] @ Callback user data pointer.

    @ TODO.
    @ TODO. [detail]
    method SetWriter
        arg Options {Options} [this] @ Target options.
        arg Callback {WriteCallback} @ Callback function.
        arg Data {Data} [userdata] @ Callback user data.

)";

std::string str = R"(@ Code.
@ Enumeration of codes for results, warnings and errors. [detail]
enum Result [errorcode]
    const Success [noerror] @ Operation completed successfully.

    const W1001 : 1001 [noerror,tokenizer(0)] @ Missing 'author' attribute
    const W1002 [noerror,tokenizer(0)] @ Missing 'copyright' attribute

    const E2001 : 2001 [tokenizer(0)] @ Unexpected character
    const E2002 [tokenizer(0)] @ TODO:
    const E2003 [tokenizer(0)] @ TODO:
    const E2004 [tokenizer(0)] @ TODO:
    const E2005 [tokenizer(0)] @ TODO:
    const E2006 [tokenizer(0)] @ TODO:
    const E2007 [tokenizer(0)] @ TODO:
    const E2008 [tokenizer(0)] @ TODO:
    const E2009 [tokenizer(0)] @ TODO:
    const E2010 [tokenizer(0)] @ TODO:
    const E2011 [tokenizer(0)] @ TODO:
    const E2012 [tokenizer(0)] @ TODO:
    const E2013 [tokenizer(0)] @ TODO:
    const E2014 [tokenizer(0)] @ TODO:
    const E2015 [tokenizer(0)] @ TODO:
    const E2016 [tokenizer(0)] @ TODO:
    const E2017 [tokenizer(0)] @ TODO:
    const E2018 [tokenizer(0)] @ TODO:
    const E2019 [tokenizer(0)] @ TODO:
    const E2020 [tokenizer(0)] @ TODO:
    const E2021 [tokenizer(0)] @ TODO:
    const E2022 [tokenizer(0)] @ TODO:
    const E2023 [tokenizer(0)] @ TODO:
    const E2024 [tokenizer(0)] @ TODO:
    const E2025 [tokenizer(0)] @ TODO:
    const E2026 [tokenizer(0)] @ TODO:
    const E2027 [tokenizer(0)] @ TODO:
    const E2028 [tokenizer(0)] @ TODO:
    const E2029 [tokenizer(0)] @ TODO:
    const E2030 [tokenizer(0)] @ TODO:
    const E2031 [tokenizer(0)] @ TODO:
    const E2032 [tokenizer(0)] @ TODO:
    const E2033 [tokenizer(0)] @ TODO:
    const E2034 [tokenizer(0)] @ TODO:
    const E2035 [tokenizer(0)] @ TODO:
    const E2036 [tokenizer(0)] @ TODO:
    const E2037 [tokenizer(0)] @ TODO:
    const E2038 [tokenizer(0)] @ TODO:
    const E2039 [tokenizer(0)] @ TODO:
    const E2040 [tokenizer(0)] @ TODO:
    const E2041 [tokenizer(0)] @ TODO:
    const E2042 [tokenizer(0)] @ TODO:
    const E2043 [tokenizer(0)] @ TODO:
    const E2044 [tokenizer(0)] @ TODO:
    const E2045 [tokenizer(0)] @ TODO:
    const E2046 [tokenizer(0)] @ TODO:
    const E2047 [tokenizer(0)] @ TODO:
    const E2048 [tokenizer(0)] @ TODO:
    const E2049 [tokenizer(0)] @ TODO:
    const E2050 [tokenizer(0)] @ TODO:
    const E2051 [tokenizer(0)] @ TODO:
    const E2052 [tokenizer(0)] @ TODO:
    const E2053 [tokenizer(0)] @ TODO:
    const E2054 [tokenizer(0)] @ TODO:
    const E2055 [tokenizer(0)] @ TODO:
    const E2056 [tokenizer(0)] @ TODO:
    const E2057 [tokenizer(0)] @ TODO:
    const E2058 [tokenizer(0)] @ TODO:
    const E2059 [tokenizer(0)] @ TODO:
    const E2060 [tokenizer(0)] @ TODO:
    const E2061 [tokenizer(0)] @ TODO:
    const E2062 [tokenizer(0)] @ TODO:
    const E2063 [tokenizer(0)] @ TODO:
    const E2064 [tokenizer(0)] @ TODO:
    const E2065 [tokenizer(0)] @ TODO:
    const E2066 [tokenizer(0)] @ TODO:
    const E2067 [tokenizer(0)] @ TODO:
    const E2068 [tokenizer(0)] @ TODO:
    const E2069 [tokenizer(0)] @ TODO:
    const E2070 [tokenizer(0)] @ TODO:
    const E2071 [tokenizer(0)] @ TODO:
    const E2072 [tokenizer(0)] @ TODO:
    const E2073 [tokenizer(0)] @ TODO:
    const E2074 [tokenizer(0)] @ TODO:
    const E2075 [tokenizer(0)] @ TODO:
    const E2076 [tokenizer(0)] @ TODO:
    const E2077 [tokenizer(0)] @ TODO:
    const E2078 [tokenizer(0)] @ TODO:
    const E2079 [tokenizer(0)] @ TODO:
    const E2080 [tokenizer(0)] @ TODO:
    const E2081 [tokenizer(0)] @ TODO:
    const E2082 [tokenizer(0)] @ TODO:
    const E2083 [tokenizer(0)] @ TODO:
    const E2084 [tokenizer(0)] @ TODO:
    const E2085 [tokenizer(0)] @ TODO:
    const E2086 [tokenizer(0)] @ TODO:
    const E2087 [tokenizer(0)] @ TODO:
    const E2088 [tokenizer(0)] @ TODO:
    const E2089 [tokenizer(0)] @ TODO:
    const E2090 [tokenizer(0)] @ TODO:
    const E2091 [tokenizer(0)] @ TODO:
    const E2092 [tokenizer(0)] @ TODO:
    const E2093 [tokenizer(0)] @ TODO:
    const E2094 [tokenizer(0)] @ TODO:
    const E2095 [tokenizer(0)] @ TODO:
    const E2096 [tokenizer(0)] @ TODO:
    const E2097 [tokenizer(0)] @ TODO:
    const E2098 [tokenizer(0)] @ TODO:
    const E2099 [tokenizer(0)] @ TODO:
    const E2100 [tokenizer(0)] @ TODO:
    const E2101 [tokenizer(0)] @ TODO:
    const E2102 [tokenizer(0)] @ TODO:
    const E2103 [tokenizer(0)] @ TODO:
    const E2104 [tokenizer(0)] @ TODO:
    const E2105 [tokenizer(0)] @ TODO:
    const E2106 [tokenizer(0)] @ TODO:
    const E2107 [tokenizer(0)] @ TODO:
    const E2108 [tokenizer(0)] @ TODO:
    const E2109 [tokenizer(0)] @ TODO:
    const E2110 [tokenizer(0)] @ TODO:
    const E2111 [tokenizer(0)] @ TODO:
    const E2112 [tokenizer(0)] @ TODO:
    const E2113 [tokenizer(0)] @ TODO:

@ TODO.
@ TODO. [detail]
struct Error
    field Code {Uint32} @ TODO.
    field Message {Str} @ TODO.

@ TODO.
@ TODO. [detail]
struct Warning
    field Code {Uint32} @ TODO.
    field Message {Str} @ TODO.

@ TODO.
@ TODO. [detail]
interface CompilationResult
    prop Status [get(GetStatus)] @ TODO.
    prop Errors [get(GetErrors)] @ TODO.
    prop Warnings [get(GetWarnings)] @ TODO.

    @ Increments reference count.
    @ Manages compilation result instance lifetime. [detail]
    @ Reference to same compilation result. [return]
    @ {Destroy} [see]
    method Reference {CompilationResult} [ref]
        arg CompilationResult {CompilationResult} [this] @ Target compilation result instance.

    @ Releases compilation result instance.
    @ Destroys when reference count reaches zero. [detail]
    @ {Reference} [see]
    method Destroy [destroy]
        arg CompilationResult {CompilationResult} [this] @ Compilation result to destroy.

    @ TODO.
    @ TODO. [detail]
    @ TODO. [return]
    method GetStatus {Result} [const]
        arg Compiler {CompilationResult} [this] @ TODO.

    @ TODO.
    @ TODO. [detail]
    method GetErrors [const]
        arg Compiler {CompilationResult} [this] @ TODO.
        arg ErrorCount {Uint32} [in,out] @ TODO.
        arg Errors {Error} [result,array(ErrorCount)] @ TODO.

    @ TODO.
    @ TODO. [detail]
    method GetWarnings [const]
        arg Compiler {CompilationResult} [this] @ TODO.
        arg WarningCount {Uint32} [in,out] @ TODO.
        arg Warnings {Error} [result,array(WarningCount)] @ TODO.

)";

idl_source_t* import(idl_utf8_t name, idl_uint32_t depth, idl_data_t data) {
    std::string s("results");
    if (name == s) {
        return new idl_source_t{ name, str.c_str(), (idl_uint32_t) str.length() };
    }
    return {};
}

void releaseImport(idl_source_t* source, idl_data_t data) {
    delete source;
}

void write(const idl_source_t* source, idl_data_t data) {
}

int main(int argc, char* argv[]) {
    auto currentPath    = std::filesystem::current_path().string();
    auto currentPathPtr = currentPath.c_str();

    idl_options_t options{};
    idl_options_create(&options);
    idl_options_set_debug_mode(options, 0);
    idl_options_set_warnings_as_errors(options, 1);
    idl_options_set_output_dir(options, "D:\\Development\\Projects\\idlc\\include\\idlc\\");
    idl_options_set_import_dirs(options, 1, &currentPathPtr);
    idl_options_set_importer(options, import, nullptr);
    idl_options_set_release_import(options, releaseImport, nullptr);
    idl_options_set_writer(options, write, nullptr);

    idl_source_t source1{ "Test", s, std::size(s) - 1 };
    idl_source_t source2{ "Options", s2, std::size(s2) - 1 };
    std::array sources = { source1, source2 };

    idl_compiler_t compiler{};
    auto res = idl_compiler_create(&compiler);

    idl_compilation_result_t result{};
    res = idl_compiler_compile(compiler,
                               IDL_GENERATOR_C,
                               "D:\\Development\\Projects\\idlc\\specs\\api.idl",
                               int(sources.size()),
                               sources.data(),
                               options,
                               &result);

    // idl_compiler_set_debug_mode(compiler, 1);

    idl_compiler_destroy(compiler);
    idl_options_destroy(options);

    return 0;
    auto input  = std::filesystem::path();
    auto output = std::filesystem::current_path();
    std::string apiver;

    argparse::ArgumentParser program("idlc", idl_version_string());
    program.add_argument("input").store_into(input).help("input .idl file");
    program.add_argument("-o", "--output").store_into(output).help("output directory");
    program.add_argument("--apiver").store_into(apiver).help("api version");
    addGeneratorArg(program);
#ifdef YYDEBUG
    auto debug = false;
    program.add_argument("-d", "--debug").store_into(debug).help("enable debugging");
#endif

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    std::optional<idl::ApiVersion> version{};
    if (program.is_used("--apiver")) {
        std::regex pattern("^([0-9]+)\\.([0-9]+)\\.([0-9]+)");
        std::smatch matches;
        if (std::regex_match(apiver, matches, pattern)) {
            try {
                version = idl::ApiVersion{ std::stoi(matches[1].str()),
                                           std::stoi(matches[2].str()),
                                           std::stoi(matches[3].str()) };
            } catch (const std::exception& e) {
                std::cerr << "invalid version number (out of range)";
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "invalid version format";
            return EXIT_FAILURE;
        }
    }

    idl::Context context{};
    idl::Scanner scanner{ context, nullptr, {}, input };
    idl::Parser parser{ scanner };
#ifdef YYDEBUG
    parser.set_debug_level(debug ? 1 : 0);
#endif
    auto code = parser.parse();

    if (code != 0) {
        return code;
    }

    context.prepareEnumConsts();
    context.prepareStructs();
    context.prepareCallbacks();
    context.prepareFunctions();
    context.prepareMethods();
    context.prepareProperties();
    context.prepareEvents();
    context.prepareInterfaces();
    context.prepareHandles();
    context.prepareDocumentation();
    context.apiVersion(version);

    switch (getGeneratorArg(program)) {
        case GeneratorType::C: {
            generateC(context, output);
            break;
        }
        case GeneratorType::Cpp: {
            generateC(context, output);
            // generateCpp(context, output);
            break;
        }
    }

    return EXIT_SUCCESS;
}
