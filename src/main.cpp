#include <argparse/argparse.hpp>

#include "parser.hpp"
#include "scanner.hpp"
#include "version.hpp"

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

int main(int argc, char* argv[]) {
    auto input  = std::filesystem::path();
    auto output = std::filesystem::current_path();

    argparse::ArgumentParser program("idlc", IDLC_VERSION_STRING);
    program.add_argument("input").store_into(input).help("input .idl file");
    program.add_argument("-o", "--output").store_into(output).help("output directory");
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

    idl::Context context{};
    idl::Scanner scanner{ context, input };
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
    context.prepareFunctions();
    context.prepareMethods();
    context.prepareProperties();
    context.prepareHandles();

    switch (getGeneratorArg(program)) {
        case GeneratorType::C: {
            generateC(context, output);
            break;
        }
        case GeneratorType::Cpp: {
            break;
        }
    }

    return EXIT_SUCCESS;
}
