#include <idlc/idl.h>

#include <argparse/argparse.hpp>
#include <regex>

void addGeneratorArg(argparse::ArgumentParser& program, const std::map<std::string, idl_generator_t>& generators) {
    auto& arg = program.add_argument("-g", "--generator");
    std::ostringstream help;
    help << "generator programming language (";
    bool first = true;
    for (auto& [gen, _] : generators) {
        arg.add_choice(gen);
        if (!first) {
            help << ", ";
        }
        first = false;
        help << gen;
    }
    help << ')';
    arg.help(help.str());
}

idl_generator_t getGeneratorArg(argparse::ArgumentParser& program,
                                const std::map<std::string, idl_generator_t>& generators) {
    if (!program.is_used("--generator")) {
        return IDL_GENERATOR_C;
    }
    std::string gen = program.get("--generator");
    return generators.at(gen);
}

int main(int argc, char* argv[]) {
    auto warnAsErr = false;
    auto input     = std::filesystem::path();
    auto output    = std::filesystem::current_path();
    auto imports   = std::vector<std::string>();
    auto additions = std::vector<std::string>();
    std::string apiver;

    std::map<std::string, idl_generator_t> generators = {
        { "c",  IDL_GENERATOR_C           },
        { "js", IDL_GENERATOR_JAVA_SCRIPT }
    };

    argparse::ArgumentParser program("idlc", IDL_VERSION_STRING);
    program.add_argument("input").store_into(input).help("input .idl file");
    addGeneratorArg(program, generators);
    program.add_argument("-o", "--output").store_into(output).help("output directory");
    program.add_argument("-i", "--imports").append().store_into(imports).help("import directories");
    program.add_argument("-a", "--additions").append().store_into(additions).help("additional inclusions");
    program.add_argument("-w", "--warnings").store_into(warnAsErr).help("warnings as errors");
    program.add_argument("--apiver").store_into(apiver).help("api version");

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
            std::cerr << "invalid version format" << std::endl;
            return EXIT_FAILURE;
        }
    }
    idl_generator_t gen   = getGeneratorArg(program, generators);
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
    idl_options_set_debug_mode(options, 0);
    idl_options_set_warnings_as_errors(options, warnAsErr ? 1 : 0);
    idl_options_set_output_dir(options, outputDir.c_str());
    idl_options_set_import_dirs(options, (idl_uint32_t) dirs.size(), dirs.data());
    idl_options_set_additions(options, (idl_uint32_t) adds.size(), adds.data());
    idl_options_set_version(options, version ? &version.value() : nullptr);

    idl_compiler_t compiler{};
    idl_compiler_create(&compiler);
    if (code != IDL_RESULT_SUCCESS) {
        idl_options_destroy(options);
        std::cerr << idl_result_to_string(code) << std::endl;
        return EXIT_FAILURE;
    }
    idl_compilation_result_t result{};
    code = idl_compiler_compile(compiler, gen, inputFile.c_str(), 0, nullptr, options, &result);

    bool failed = false;
    if (result) {
        if (idl_compilation_result_has_errors(result)) {
            failed = true;
        }
        if (idl_compilation_result_has_errors(result) || idl_compilation_result_has_warnings(result)) {
            idl_uint32_t count{};
            idl_compilation_result_get_messages(result, &count, nullptr);
            std::vector<idl_message_t> messages;
            messages.resize(count);
            idl_compilation_result_get_messages(result, &count, messages.data());
            for (const auto& message : messages) {
                std::cerr << (message.is_error ? "error" : "warning");
                std::cerr << " [" << (message.status >= IDL_STATUS_E2001 ? 'E' : 'W');
                std::cerr << (int) message.status << "]: " << message.message;
                if (message.line > 0) {
                    std::cerr << " at " << message.filename << ':' << message.line << ':' << message.column << '.'
                              << std::endl;
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
