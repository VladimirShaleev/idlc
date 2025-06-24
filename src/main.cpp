#include <idlc/idl.h>

#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <regex>

void addGeneratorArg(argparse::ArgumentParser& program) {
    auto& arg = program.add_argument("-g", "--generator");
    std::ostringstream help;
    help << "generator programming language (";
    bool first = true;
    for (auto& gen : magic_enum::enum_names<idl_generator_t>()) {
        auto str = std::string(gen.data()).substr(14);
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

idl_generator_t getGeneratorArg(argparse::ArgumentParser& program) {
    if (!program.is_used("--generator")) {
        return IDL_GENERATOR_C;
    }
    auto str = "idl_generator_" + program.get("--generator");
    return magic_enum::enum_cast<idl_generator_t>(str, magic_enum::case_insensitive).value();
}

int main(int argc, char* argv[]) {
    auto input   = std::filesystem::path();
    auto output  = std::filesystem::current_path();
    auto imports = std::vector<std::string>();
    std::string apiver;

    argparse::ArgumentParser program("idlc", IDL_VERSION_STRING);
    program.add_argument("input").store_into(input).help("input .idl file");
    program.add_argument("-o", "--output").store_into(output).help("output directory");
    program.add_argument("--apiver").store_into(apiver).help("api version");
    program.add_argument("-i", "--imports").append().store_into(imports).help("import dirs");
    addGeneratorArg(program);

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
                std::cerr << "invalid version number (out of range)";
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "invalid version format";
            return EXIT_FAILURE;
        }
    }
    std::string inputFile = input.string();
    std::string outputDir = output.string();
    std::vector<idl_utf8_t> dirs;
    for (const auto& import : imports) {
        dirs.push_back(import.c_str());
    }

    idl_options_t options{};
    idl_options_create(&options);
    idl_options_set_debug_mode(options, 0);
    // idl_options_set_warnings_as_errors(options, 1);
    idl_options_set_output_dir(options, outputDir.c_str());
    idl_options_set_import_dirs(options, (idl_uint32_t) dirs.size(), dirs.data());
    idl_options_set_version(options, version ? &version.value() : nullptr);

    idl_compiler_t compiler{};
    auto res = idl_compiler_create(&compiler);

    idl_compilation_result_t result{};
    res = idl_compiler_compile(compiler, getGeneratorArg(program), inputFile.c_str(), 0, nullptr, options, &result);

    idl_compiler_destroy(compiler);
    idl_options_destroy(options);

    return EXIT_SUCCESS;
}
