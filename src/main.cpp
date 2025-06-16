#include <argparse/argparse.hpp>

#include "parser.hpp"
#include "scanner.hpp"
#include "version.hpp"
#include "visitors.hpp"

int main(int argc, char* argv[]) {
    auto input  = std::filesystem::path();
    auto output = std::filesystem::current_path();

    argparse::ArgumentParser program("idlc", IDLC_VERSION_STRING);
    program.add_argument("input").store_into(input).help("input .idl file");
    program.add_argument("-o", "--output").store_into(output).help("output directory");
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

    context.calcEnumConsts();

    return EXIT_SUCCESS;
}
