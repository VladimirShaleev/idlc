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
    #define intern(loc, str) \
        scanner.context().intern<idl::Parser::syntax_error>(loc, str)
    
    void addDoc(const idl::location&, ASTDoc*, const std::vector<ASTNode*>&, char);
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

%token ATTRFLAGS

%token API
%token ENUM

%token <std::string> STR
%token <std::string> ID
%token <int64_t> NUM

%type <ASTAttr*> attr_item
%type <std::vector<ASTAttr*>> attr_list

%type <ASTDoc*> doc
%type <ASTNode*> doc_lit_or_ref
%type <std::vector<ASTNode*>> doc_field
%type <std::pair<std::vector<ASTNode*>, char>> doc_decl

%type <ASTApi*> root
%type <ASTApi*> api_def

%type <ASTEnum*> enum
%type <ASTEnum*> enum_def

%start root

%%

root
    : api_def { $$ = $1; }
    | enum { throw syntax_error(@1, err_str<E2012>()); }
    | root api_def { throw syntax_error(@2, err_str<E2004>()); }
    | root enum { $1->enums.push_back($2); $2->parent = $1; $$ = $1; }
    ;

api_def
    : API ID { throw syntax_error(@1, err_str<E2005>()); }
    | doc API ID { 
        auto node = alloc_node(ASTApi, @2);
        node->name = $3;
        node->doc = $1;
        $1->parent = node;
        $$ = node;
        }
    ;

enum
    : enum_def { $$ = $1; }
    | enum_def attr_list { 
        $1->attrs = $2; 
        for (auto attr : $1->attrs) {
            attr->parent = $1;
        }
        $$ = $1;
    }
    ;

enum_def
    : ENUM ID { throw syntax_error(@1, err_str<E2005>()); }
    | doc ENUM ID {
        auto node = alloc_node(ASTEnum, @2);
        node->name = $3;
        node->doc = $1;
        $1->parent = node;
        $$ = node;
    }
    ;

attr_list
    : attr_item { auto list = std::vector<ASTAttr*>(); list.push_back($1); $$ = list; }
    | attr_list ',' attr_item { $1.push_back($3); $$ = $1; }
    ;

attr_item
    : ATTRFLAGS { auto node = alloc_node(ASTAttr, @1); node->type = ASTAttr::Flags; $$ = node; }
    ;

doc
    : doc_decl { auto node = alloc_node(ASTDoc, @1); addDoc(@1, node, $1.first, $1.second); $$ = node; }
    | doc doc_decl { addDoc(@2, $1, $2.first, $2.second); $$ = $1; }
    ;

doc_decl
    : DOC { throw syntax_error(@$, err_str<E2006>()); }
    | DOC doc_field              { $$ = std::make_pair($2, 'b'); }
    | DOC doc_field DOCBRIEF     { $$ = std::make_pair($2, 'b'); }
    | DOC doc_field DOCDETAIL    { $$ = std::make_pair($2, 'd'); }
    | DOC doc_field DOCCOPYRIGHT { $$ = std::make_pair($2, 'c'); }
    | DOC doc_field DOCLICENSE   { $$ = std::make_pair($2, 'l'); }
    | DOC doc_field DOCAUTHOR    { $$ = std::make_pair($2, 'a'); }
    ;

doc_field
    : doc_lit_or_ref { auto list = std::vector<ASTNode*>(); list.push_back($1); $$ = list; }
    | doc_field doc_lit_or_ref { $1.push_back($2); $$ = $1; }
    ;

doc_lit_or_ref
    : STR { $$ = intern(@1, $1); }
    | '{' STR '}' { auto node = alloc_node(ASTDeclRef, @2); node->name = $2; $$ = node; }
    ;

%%

void addDoc(const idl::location& loc, ASTDoc* node, const std::vector<ASTNode*>& doc, char type) {
    switch (type) {
        case 'b':
            if (!node->brief.empty()) throw idl::Parser::syntax_error(loc, err_str<E2007>());
            node->brief = doc;
            break;
        case 'd':
            if (!node->detail.empty()) throw idl::Parser::syntax_error(loc, err_str<E2008>());
            node->detail = doc;
            break;
        case 'c':
            if (!node->copyright.empty()) throw idl::Parser::syntax_error(loc, err_str<E2009>());
            node->copyright = doc;
            break;
        case 'l':
            if (!node->license.empty()) throw idl::Parser::syntax_error(loc, err_str<E2010>());
            node->license = doc;
            break;
        case 'a':
            node->authors.push_back(doc);
            break;
    }
}

void idl::Parser::error(const location_type& loc, const std::string& message)
{
   std::cerr << "error: " << message << " at " << Location(loc) << std::endl;
}
