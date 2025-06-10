%require "3.7.4"
%language "C++"
%defines "parser.hpp"
%output "parser.cpp"
%locations

%{
#include <iostream>
#include <string>
#include <cmath>
#include <FlexLexer.h>
%}

%define api.parser.class {Parser}
%define api.namespace {idl}
%define api.value.type variant
%parse-param {Scanner& scanner}

%code requires
{
    #include <optional>
    #include "ast.hpp"

    namespace idl {
        class Scanner;
    }
}

%code
{
    #include "scanner.hpp"
    #define yylex scanner.yylex
    #define alloc_node(ast, loc) \
        scanner.context().allocNode<ast, idl::Parser::syntax_error>(loc);
}

%initial-action {
    @$.begin.filename = @$.end.filename = scanner.filename();
}

%token NAMESPACE
%token ENUM
%token INTERFACE
%token IN OUT
%token INT
%token <std::string> ID
%token <int64_t> NUM

%type <ASTNode*> program
%type <ASTNamespace*> namespace_decl
%type <ASTDecl*> enum_decl
%type <ASTDecl*> interface_decl
%type <ASTDecl*> method_decl
%type <ASTDecl*> param_decl
%type <std::vector<ASTDecl*>> declarations
%type <std::vector<ASTNode*>> method_list
%type <std::vector<ASTNode*>> param_list
%type <std::pair<std::string, std::optional<int64_t>>> enum_const
%type <std::vector<std::pair<std::string, int64_t>>> enum_const_list
%type <std::string> type
%type <std::string> direction

%start program

%%

program : 
    namespace_decl { 
        auto program = alloc_node(ASTProgram, @1);
        program->namespaces.push_back($1);
        $1->parent = program;
        $$ = program;
    }
    ;

namespace_decl : 
    NAMESPACE ID '{' declarations '}' {
        auto ns = alloc_node(ASTNamespace, @2);
        ns->name = $2;
        for (auto decl : $4) {
            ns->declarations.push_back(decl);
            decl->parent = ns;
        }
        $$ = ns;
    }
    ;

declarations : 
    /* empty */ { $$ = std::vector<ASTDecl*>(); }
    | declarations enum_decl { $1.push_back($2); $$ = $1; }
    | declarations interface_decl { $1.push_back($2); $$ = $1; }
    ;

enum_decl : 
    ENUM ID '{' enum_const_list '}' {
        auto en = alloc_node(ASTEnum, @2);
        en->name = $2;
        for (auto [name, value] : $4) {
            auto ec = alloc_node(ASTEnumConst, @4);
            ec->name = name;
            ec->value = value;
            ec->parent = en;
            en->constants.push_back(ec);
        }
        $$ = en;
    }
    ;

enum_const_list : 
    enum_const { 
        auto list = std::vector<std::pair<std::string, int64_t>>();
        list.emplace_back($1.first, $1.second.value_or(0));
        $$ = list;
    }
    | enum_const_list ',' enum_const {
        $1.emplace_back($3.first, $3.second.value_or($1.back().second + 1));
        $$ = $1;
    }
    ;

enum_const : 
    ID { 
        auto p = std::pair<std::string, std::optional<int64_t>>($1, std::nullopt);
        $$ = p;
    }
    | ID '=' NUM { 
        auto p = std::pair<std::string, std::optional<int64_t>>($1, $3);
        $$ = p;
    }
    ;

interface_decl : 
    INTERFACE ID '{' '}' {
        auto iface = alloc_node(ASTInterface, @2);
        iface->name = $2;
        // for (auto method : *$4) {
        //     iface->methods.push_back(std::unique_ptr<ASTNode>(method));
        // }
        // delete $4;
        $$ = iface;
    }
    ;

type : 
    INT { $$ = new std::string("int"); }
    ;

%%

void idl::Parser::error(const location_type& loc, const std::string& message)
{
   std::cerr << "error: " << message << " at " << Location(loc) << std::endl;
}
