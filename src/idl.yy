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
%type <ASTNode*> namespace_decl
%type <ASTNode*> enum_decl
%type <ASTNode*> interface_decl
%type <ASTNode*> method_decl
%type <ASTNode*> param_decl
%type <std::vector<ASTNode*>> declarations
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
        if (!scanner.program) {
            scanner.program = new ASTProgram();
        }
        scanner.program->namespaces.push_back(std::unique_ptr<ASTNode>($1));
        $$ = scanner.program;
    }
    ;

namespace_decl : 
    NAMESPACE ID '{' declarations '}' {
        auto ns = new ASTNamespace();
        ns->name = $2;
        for (auto decl : $4) {
            ns->declarations.push_back(std::unique_ptr<ASTNode>(decl));
        }
        $$ = ns;
    }
    ;

declarations : 
    /* empty */ { $$ = std::vector<ASTNode*>(); }
    | declarations enum_decl { $1.push_back($2); $$ = $1; }
    | declarations interface_decl { $1.push_back($2); $$ = $1; }
    ;

enum_decl : 
    ENUM ID '{' enum_const_list '}' {
        auto en = new ASTEnum();
        en->name = $2;
        for (auto [name, value] : $4) {
            en->constants.emplace_back(name, value);
        }
        $$ = en;
    }
    ;

enum_const_list : 
    enum_const { 
        auto list = std::vector<std::pair<std::string, int64_t>>();
        list.emplace_back($1.first, $1.second ? $1.second.value() : 0);
        $$ = list;
    }
    | enum_const_list ',' enum_const {
        $1.emplace_back($3.first, $3.second ? $3.second.value() : $1.back().second + 1);
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
        auto iface = new ASTInterface();
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
   std::cerr << "error: " << message << " at " << loc << std::endl;
}
