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
    idl_options_set_writer(options, write, nullptr);

    idl_compiler_t compiler{};
    auto res = idl_compiler_create(&compiler);

    idl_compilation_result_t result{};
    res = idl_compiler_compile(
        compiler, IDL_GENERATOR_C, "D:\\Development\\Projects\\idlc\\specs\\api.idl", 0, nullptr, options, &result);

    idl_compiler_destroy(compiler);
    idl_options_destroy(options);

    return EXIT_SUCCESS;
}
