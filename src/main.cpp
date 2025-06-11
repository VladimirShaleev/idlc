#include <argparse/argparse.hpp>
#include <fstream>

#include "parser.hpp"
#include "scanner.hpp"
#include "version.hpp"

struct PrintIDL : Visitor {
    bool visit(const ASTDeclRef* node) override {
        std::cout << node->decl->type();
        return true;
    }

    bool visit(const ASTAttribute* node) override {
        switch (node->type) {
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
        if (!node->arguments.empty()) {
            std::cout << '(';
            auto hasPrevArg = false;
            for (auto arg : node->arguments) {
                if (hasPrevArg) {
                    std::cout << ',';
                }
                std::cout << arg;
                hasPrevArg = true;
            }
            std::cout << ')';
        }
        return true;
    }

    bool visit(const ASTProgram* node) override {
        auto hasPrev = false;
        for (auto ns : node->namespaces) {
            if (hasPrev) {
                std::cout << std::endl;
            }
            ns->accept(*this);
            hasPrev = true;
        }
        return true;
    }

    bool visit(const ASTNamespace* node) override {
        printAttributes(node);
        std::cout << "namespace " << node->name << std::endl;
        std::cout << '{' << std::endl;
        if (!node->declarations.empty()) {
            for (auto decl : node->declarations) {
                std::cout << std::endl;
                decl->accept(*this);
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
        std::cout << '}' << std::endl;
        return true;
    }

    bool visit(const ASTEnum* node) override {
        printAttributes(node);
        std::cout << "enum " << node->name << std::endl;
        std::cout << '{' << std::endl;
        for (size_t i = 0; i < node->constants.size(); ++i) {
            node->constants[i]->accept(*this);
            if (i + 1 < node->constants.size()) {
                std::cout << ',';
            }
            std::cout << std::endl;
        }
        std::cout << '}';
        return true;
    }

    bool visit(const ASTEnumConst* node) override {
        auto hexOutput = dynamic_cast<ASTEnum*>(node->parent)->isHexOutputValues();
        std::cout << "    " << node->name << " = ";
        if (hexOutput) {
            std::cout << "0x" << std::hex << node->value << std::dec;
        } else {
            std::cout << node->value;
        }
        return true;
    }

    bool visit(const ASTInterface* node) override {
        printAttributes(node);
        std::cout << "interface " << node->name << std::endl;
        std::cout << '{' << std::endl;
        for (auto method : node->methods) {
            method->accept(*this);
        }
        std::cout << '}';
        return true;
    }

    bool visit(const ASTMethod* node) override {
        std::cout << "    ";
        node->returnTypeRef->accept(*this);
        std::cout << ' ' << node->name << '(';
        for (size_t i = 0; i < node->parameters.size(); ++i) {
            auto param = node->parameters[i];
            param->typeRef->accept(*this);
            std::cout << ' ' << param->name;
            if (i + 1 < node->parameters.size()) {
                std::cout << ", ";
            }
        }
        std::cout << ");" << std::endl;
        return true;
    }

    bool visit(const ASTParameter* node) override {
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
                attr->accept(*this);
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
    context.program()->accept(printIDL);
    return EXIT_SUCCESS;
}
