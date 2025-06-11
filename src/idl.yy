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
        scanner.context().allocNode<ast, idl::Parser::syntax_error>(loc)
}

%initial-action {
    @$.begin.filename = @$.end.filename = scanner.filename();
}

%token DOC
%token DOCBRIEF
%token DOCDETAIL
%token DOCCOPYRIGHT
%token DOCLICENSE
%token DOCAUTHOR
%token API
%token <std::string> STR
%token <std::string> ID
%token <int64_t> NUM

%type <ASTNode*> api

%type <ASTDoc*> doc
%type <ASTNode*> doc_item
%type <std::vector<ASTNode*>> doc_field
%type <std::vector<ASTNode*>> doc_list

%start root

%%

root
    : api
    | root api

api
    : API ID { throw syntax_error(@1, err_str<E2005>()); }
    | doc API ID { auto node = alloc_node(ASTApi, @2); node->name = $3; node->doc = $1; $$ = node; }
    ;

doc
    : { auto node = alloc_node(ASTDoc, @$); $$ = node; }
    | doc doc_field { 
        if (!$1->brief.empty()) throw syntax_error(@2, err_str<E2007>());
        $1->brief = $2; $$ = $1; 
    }
    | doc doc_field DOCBRIEF { 
        if (!$1->brief.empty()) throw syntax_error(@2, err_str<E2007>());
        $1->brief = $2; $$ = $1; 
    }
    | doc doc_field DOCDETAIL { 
        if (!$1->detail.empty()) throw syntax_error(@2, err_str<E2008>());
        $1->detail = $2; $$ = $1; 
    }
    | doc doc_field DOCCOPYRIGHT { 
        if (!$1->copyright.empty()) throw syntax_error(@2, err_str<E2009>());
        $1->copyright = $2; $$ = $1; 
    }
    | doc doc_field DOCLICENSE { 
        if (!$1->license.empty()) throw syntax_error(@2, err_str<E2010>());
        $1->license = $2; $$ = $1; 
    }
    | doc doc_field DOCAUTHOR { 
        $1->authors.push_back($2); $$ = $1; 
    }
    ;

doc_field:
    DOC doc_list { $$ = $2; }
    ;

doc_list
    : doc_item { auto list = std::vector<ASTNode*>(); list.push_back($1); $$ = list; }
    | doc_list doc_item { $1.push_back($2); $$ = $1; }
    ;

doc_item
    : { throw syntax_error(@$, err_str<E2006>()); }
    | STR { auto node = alloc_node(ASTLiteralStr, @1); node->value = $1; $$ = node; }
    | '{' STR '}' { auto node = alloc_node(ASTDeclRef, @1); node->name = $2; $$ = node; }
    ;

%%

void idl::Parser::error(const location_type& loc, const std::string& message)
{
   std::cerr << "error: " << message << " at " << Location(loc) << std::endl;
}
