#include <argparse/argparse.hpp>
#include <fstream>

#include "parser.hpp"
#include "scanner.hpp"
#include "version.hpp"
#include "visitors.hpp"

struct PrintIDL : HierarchyState,
                  Visitor {
    bool visit(const ASTNamespace* node) override {
        const auto parentIsProgram = dynamic_cast<const ASTProgram*>(node->parent) != nullptr;
        if (state == Begin) {
            if (hasPrev == parentIsProgram) {
                std::cout << std::endl;
            }
            printAttributes(node);
            std::cout << "namespace " << node->name << std::endl;
            std::cout << '{' << std::endl;
        } else if (state == End) {
            std::cout << '}' << std::endl;
            if (hasPrev != parentIsProgram) {
                std::cout << std::endl;
            }
        }
        return true;
    }

    bool visit(const ASTEnum* node) override {
        if (state == Begin) {
            if (!hasPrev) {
                std::cout << std::endl;
            }
            printAttributes(node);
            std::cout << "enum " << node->name << std::endl;
            std::cout << '{' << std::endl;
        } else if (state == End) {
            std::cout << '}' << std::endl << std::endl;
        }
        return true;
    }

    bool visit(const ASTEnumConst* node) override {
        if (state == Begin) {
            const auto hexOutput = dynamic_cast<ASTEnum*>(node->parent)->isHexOutputValues();
            std::cout << "    " << node->name << " = ";
            if (hexOutput) {
                std::cout << "0x" << std::hex << node->value << std::dec;
            } else {
                std::cout << node->value;
            }
            if (hasNext) {
                std::cout << ',';
            }
            std::cout << std::endl;
        }
        return true;
    }

    bool visit(const ASTInterface* node) override {
        if (state == Begin) {
            if (!hasPrev) {
                std::cout << std::endl;
            }
            printAttributes(node);
            std::cout << "interface " << node->name << std::endl;
            std::cout << '{' << std::endl;
        } else if (state == End) {
            std::cout << '}' << std::endl << std::endl;
        }
        return true;
    }

    bool visit(const ASTMethod* node) override {
        if (state == Begin) {
            std::cout << "    " << node->returnTypeRef->decl->type() << ' ' << node->name << '(';
        } else if (state == End) {
            std::cout << ");" << std::endl;
        }
        return true;
    }

    bool visit(const ASTParameter* node) override {
        if (hasPrev) {
            std::cout << ", ";
        }
        printAttributes(node, false);
        std::cout << node->typeRef->decl->type() << ' ' << node->name;
        return false;
    }

    void printAttributes(const ASTDecl* decl, bool newLine = true) {
        if (!decl->attributes.empty()) {
            std::cout << '[';
            auto hasPrev = false;
            for (auto attr : decl->attributes) {
                if (hasPrev) {
                    std::cout << ',';
                }
                switch (attr->type) {
                    case ASTAttribute::Platform:
                        std::cout << "platform";
                        break;
                    case ASTAttribute::Flags:
                        std::cout << "flags";
                        break;
                    case ASTAttribute::Hex:
                        std::cout << "hex";
                        break;
                    case ASTAttribute::In:
                        std::cout << "in";
                        break;
                    case ASTAttribute::Out:
                        std::cout << "out";
                        break;
                }
                if (!attr->arguments.empty()) {
                    std::cout << '(';
                    auto hasPrevArg = false;
                    for (auto arg : attr->arguments) {
                        if (hasPrevArg) {
                            std::cout << ',';
                        }
                        std::cout << arg;
                        hasPrevArg = true;
                    }
                    std::cout << ')';
                }
                hasPrev = true;
            }
            std::cout << ']';
            if (newLine) {
                std::cout << std::endl;
            } else {
                std::cout << ' ';
            }
        }
    }
};

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
    auto code = parser.parse();
    file.close();

    if (code != 0) {
        return code;
    }

    if (!context.updateSymbols()) {
        return EXIT_FAILURE;
    }

    if (!context.resolveDeclRefs()) {
        return EXIT_FAILURE;
    }

    PrintIDL printIDL;
    HierarchyVisitor hierarchyVisitor(printIDL);
    context.program()->accept(hierarchyVisitor);
    return EXIT_SUCCESS;
}
