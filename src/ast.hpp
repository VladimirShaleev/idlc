#ifndef AST_HPP
#define AST_HPP

#include <algorithm>
#include <string>
#include <vector>

#include "location.hh"

struct ASTNode {
    virtual ~ASTNode() = default;
    ASTNode* parent{};
    idl::location location{};
};

struct ASTLiteral : ASTNode {};

struct ASTLiteralStr : ASTLiteral {
    std::string value;
};

struct ASTDoc : ASTNode {
    std::vector<ASTNode*> brief;
    std::vector<ASTNode*> detail;
    std::vector<ASTNode*> copyright;
    std::vector<ASTNode*> license;
    std::vector<std::vector<ASTNode*>> authors;
};

struct ASTAttr : ASTNode {
    enum Type {
        Flags
    };
    Type type;
};

struct ASTDecl : ASTNode {
    std::string name;
    std::vector<ASTAttr*> attrs;
    ASTDoc* doc{};
};

struct ASTDeclRef : ASTNode {
    std::string name;
    ASTDecl* decl{};
};

struct ASTType : ASTDecl {};

struct ASTEnum : ASTType {};

struct ASTApi : ASTDecl {
    std::vector<ASTEnum*> enums;
};

#endif
