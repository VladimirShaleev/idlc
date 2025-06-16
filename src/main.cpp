#include <argparse/argparse.hpp>
#include <fstream>

#include "parser.hpp"
#include "scanner.hpp"
#include "version.hpp"
#include "visitors.hpp"

int main(int argc, char* argv[]) {
    auto input  = std::filesystem::path();
    auto output = std::filesystem::current_path();

    argparse::ArgumentParser program("idlc", IDLC_VERSION_STRING);
    program.add_argument("-i", "--input").store_into(input).help("input .idl file");
    program.add_argument("-o", "--output").store_into(output).help("output directory");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    std::ifstream file;
    std::istream* stream = &std::cin;
    std::string filename = "<stdin>";
    if (program.is_used("--input")) {
        file = std::ifstream(input);
        if (file.fail()) {
            std::cerr << "Failed to open .idl file" << std::endl;
            return EXIT_FAILURE;
        }
        stream   = &file;
        filename = input.filename().string();
    }

    idl::Context context{};
    idl::Scanner scanner{ context, stream, &filename };
    idl::Parser parser{ scanner };
    // parser.set_debug_level(1);
    auto code = parser.parse();
    file.close();

    if (code != 0) {
        return code;
    }

    context.calcEnumConsts();

    return EXIT_SUCCESS;
}
