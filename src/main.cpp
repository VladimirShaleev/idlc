#include <idlc/idl.h>

#include <argparse/argparse.hpp>
#include <regex>

using namespace std::string_view_literals;

template <typename T, typename... Args>
std::function<T(argparse::ArgumentParser&, const std::map<std::string, T>&)>
addCommand(argparse::ArgumentParser& program, std::string_view help, T defaultValue, const std::map<std::string, T>& keys, Args&&... args) {
    auto& arg = program.add_argument(args...);
    std::ostringstream ss;
    ss << help << " ("sv;
    bool first = true;
    for (auto& [key, _] : keys) {
        arg.add_choice(key);
        if (!first) {
            ss << ", "sv;
        }
        first = false;
        ss << key;
    }
    ss << ')';
    arg.help(ss.str());

    auto lastArg = (args, ...);
    return [defaultValue, lastArg](argparse::ArgumentParser& program, const std::map<std::string, T>& keys) {
        if (!program.is_used(lastArg)) {
            return defaultValue;
        }
        std::string key = program.get(lastArg);
        return keys.at(key);
    };
}

int main(int argc, char* argv[]) {
    auto warnAsErr = false;
    auto input     = std::filesystem::path();
    auto output    = std::filesystem::current_path();
    auto imports   = std::vector<std::string>();
    auto additions = std::vector<std::string>();
    std::string apiver;
    uint32_t indents = 4;
    uint32_t line    = 120;
    bool idlOriginal;
    bool cAddDoc;
    bool cAddDocGroups;
    bool cAddMemberGroups;

    std::map<std::string, idl_generator_t> generators = {
        { "idl", IDL_GENERATOR_IDL         },
        { "c",   IDL_GENERATOR_C           },
        { "js",  IDL_GENERATOR_JAVA_SCRIPT },
        { "cs",  IDL_GENERATOR_CSHARP      }
    };

    std::map<std::string, idl_bool_type_t> boolTypes = {
        { "default", IDL_BOOL_TYPE_DEFAULT  },
        { "int32",   IDL_BOOL_TYPE_INT_32   },
        { "int8",    IDL_BOOL_TYPE_INT_8    },
        { "std",     IDL_BOOL_TYPE_STD_BOOL }
    };

    std::map<std::string, idl_trivial_types_t> trivialTypes = {
        { "default", IDL_TRIVIAL_TYPES_DEFAULT     },
        { "std",     IDL_TRIVIAL_TYPES_STD         },
        { "api",     IDL_TRIVIAL_TYPES_API_DEFINED }
    };

    std::map<std::string, idl_output_files_t> formats = {
        { "default", IDL_OUTPUT_FILES_DEFAULT },
        { "single",  IDL_OUTPUT_FILES_SINGLE  },
        { "multi",   IDL_OUTPUT_FILES_MULTI   }
    };

    argparse::ArgumentParser program("idlc", IDL_VERSION_STRING);
    program.add_argument("input").store_into(input).help("input .idl file");
    auto generator = addCommand(program, "generator programming language"sv, IDL_GENERATOR_C, generators, "-g"sv, "--generator"sv);
    program.add_argument("-o", "--output").store_into(output).help("output directory");
    program.add_argument("-i", "--imports").append().store_into(imports).help("import directories");
    program.add_argument("-w", "--warnings").store_into(warnAsErr).help("warnings as errors");
    auto boolType = addCommand(program, "bool type"sv, IDL_BOOL_TYPE_DEFAULT, boolTypes, "--bool"sv);
    auto trivials = addCommand(program, "trivial types"sv, IDL_TRIVIAL_TYPES_DEFAULT, trivialTypes, "--trivials"sv);
    auto format   = addCommand(program, "output files format"sv, IDL_OUTPUT_FILES_DEFAULT, formats, "--output-files"sv);
    program.add_argument("--apiver").store_into(apiver).help("api version");
    program.add_argument("--indents").store_into(indents).help("indents count");
    program.add_argument("--line").store_into(line).help("max line length");
    program.add_argument("--idl-original").store_into(idlOriginal).help("prefered original style");
    program.add_argument("--c-add-doc").store_into(cAddDoc).help("add Doxygen documentation");
    program.add_argument("--c-add-doc-groups").store_into(cAddDocGroups).help("add Doxygen groups");
    program.add_argument("--c-add-member-groups").store_into(cAddMemberGroups).help("add Doxygen grouping members");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    std::optional<idl_api_version_t> version{};
    if (program.is_used("--apiver")) {
        std::regex pattern("^([0-9]+)\\.([0-9]+)\\.([0-9]+)");
        std::smatch matches;
        if (std::regex_match(apiver, matches, pattern)) {
            try {
                version = idl_api_version_t{ (idl_uint32_t) std::stoi(matches[1].str()),
                                             (idl_uint32_t) std::stoi(matches[2].str()),
                                             (idl_uint32_t) std::stoi(matches[3].str()) };
            } catch (const std::exception& e) {
                std::cerr << "invalid version number (out of range)" << std::endl;
                return EXIT_FAILURE;
            }
        } else {
            version = idl_api_version_t{ .str = apiver.c_str() };
        }
    }
    std::string inputFile = input.string();
    std::string outputDir = output.string();
    std::vector<idl_utf8_t> dirs;
    std::vector<idl_utf8_t> adds;
    for (const auto& import : imports) {
        dirs.push_back(import.c_str());
    }
    for (const auto& addition : additions) {
        adds.push_back(addition.c_str());
    }

    idl_options_t options{};
    auto code = idl_options_create(&options);
    if (code != IDL_RESULT_SUCCESS) {
        std::cerr << idl_result_to_string(code) << std::endl;
        return EXIT_FAILURE;
    }
    idl_idl_options_t idlOptions{};
    idlOptions.prefered_original_style = idlOriginal ? 1 : 0;

    idl_c_options_t cOptions{};
    cOptions.add_doc           = cAddDoc ? 1 : 0;
    cOptions.add_doc_groups    = cAddDocGroups ? 1 : 0;
    cOptions.add_member_groups = cAddMemberGroups ? 1 : 0;

    idl_options_set_debug_mode(options, 0);
    idl_options_set_warnings_as_errors(options, warnAsErr ? 1 : 0);
    idl_options_set_output_dir(options, outputDir.c_str());
    idl_options_set_import_dirs(options, (idl_uint32_t) dirs.size(), dirs.data());
    idl_options_set_version(options, version ? &version.value() : nullptr);
    idl_options_set_indents(options, indents);
    idl_options_set_line_length(options, line);
    idl_options_set_output_files(options, format(program, formats));
    idl_options_set_trivial_types(options, trivials(program, trivialTypes));
    idl_options_set_bool_type(options, boolType(program, boolTypes));
    idl_options_set_idl_options(options, &idlOptions);
    idl_options_set_c_options(options, &cOptions);
    idl_compiler_t compiler{};
    idl_compiler_create(&compiler);
    if (code != IDL_RESULT_SUCCESS) {
        idl_options_destroy(options);
        std::cerr << idl_result_to_string(code) << std::endl;
        return EXIT_FAILURE;
    }
    idl_compilation_result_t result{};
    code = idl_compiler_compile(compiler, generator(program, generators), inputFile.c_str(), 0, nullptr, options, &result);

    bool failed = false;
    if (result) {
        if (idl_compilation_result_has_errors(result)) {
            failed = true;
        }
        if (idl_compilation_result_has_errors(result) || idl_compilation_result_has_warnings(result) || idl_compilation_result_has_notes(result)) {
            idl_uint32_t count{};
            idl_compilation_result_get_messages(result, &count, nullptr);
            std::vector<idl_message_t> messages;
            messages.resize(count);
            idl_compilation_result_get_messages(result, &count, messages.data());
            for (const auto& message : messages) {
                std::cerr << (message.status >= IDL_STATUS_E3001 ? "error" : (message.status >= IDL_STATUS_W2001 ? "warning" : "note"));
                std::cerr << " [" << (message.status >= IDL_STATUS_E3001 ? 'E' : (message.status >= IDL_STATUS_W2001 ? 'W' : 'N'));
                std::cerr << (int) message.status << "]: " << message.message;
                if (message.line > 0) {
                    std::cerr << " at " << message.filename << ':' << message.line << ':' << message.column << '.' << std::endl;
                }
            }
        }
        idl_compilation_result_destroy(result);
    }
    if (code != IDL_RESULT_SUCCESS) {
        std::cerr << "error: " << idl_result_to_string(code) << std::endl;
        failed = true;
    }

    idl_compiler_destroy(compiler);
    idl_options_destroy(options);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
