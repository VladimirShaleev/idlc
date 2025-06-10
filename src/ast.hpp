#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <string>
#include <vector>

struct ASTNode;
struct ASTProgram;
struct ASTNamespace;
struct ASTEnum;
struct ASTInterface;
struct ASTMethod;
struct ASTParameter;

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct ASTDecl : ASTNode {

};

struct ASTProgram : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> namespaces;
};

struct ASTNamespace : ASTDecl {
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> declarations;
};

struct ASTEnum : ASTDecl {
    struct Constant {
        std::string name;
        int64_t value;
    };

    std::string name;
    std::vector<Constant> constants;
};

struct ASTInterface : ASTDecl {
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> methods;
};

struct ASTMethod : ASTDecl {
    std::string returnType;
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> parameters;
};

struct ASTParameter : ASTDecl {
    std::string direction;
    std::string type;
    std::string name;
};

#endif
