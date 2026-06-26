%require "3.8.2"
%language "C++"
%defines "parser.hpp"
%output "parser.cpp"
%locations

%{
#include "rules.hpp"
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
    #define yytext scanner.YYText()
    #define rule(type, node, ...) \
        scanner.context().visit<type##Rules>(node __VA_OPT__(,) __VA_ARGS__)
    #define visit(type, node, result, ...) \
        scanner.context().visit<type>(node __VA_OPT__(,) __VA_ARGS__).result
    #define alloc_node(ast, loc) \
        scanner.context().allocNode<AST##ast>(loc)
    #define add_literal(loc, value) \
        scanner.context().addLiteral(loc, value)
    #define add_symbol(decl) \
        scanner.context().addSymbol(decl)
    #define log(status, loc, ...) \
        scanner.context().log<IDL_STATUS_##status>(loc __VA_OPT__(,) __VA_ARGS__)
}

%initial-action {
    @$.begin.filename = @$.end.filename = scanner.filename();
}

%token API
%token ENUM
%token CONST
%token IMPORT
%token POPIMPORT
%token DOC
%token IDOC

%token ATTRVERSION
%token ATTRAUTHOR
%token ATTRCOPYRIGHT
%token ATTRLICENSE
%token ATTRFLAGS
%token ATTRHEX
%token ATTRBRIEF
%token ATTRDETAIL
%token ATTRVALUE
%token ATTRTYPE
%token ATTRCNAME
%token ATTRTOKENIZER
%token ATTRORDER
%token ATTRSINGLE

%token <std::string> ID
%token <int64_t>     INT
%token <double>      FLOAT
%token <bool>        BOOL
%token <std::string> STR
%token <std::string> INVALID_ARG
%token <std::string> INVALID_ATTR

%type <ASTDecl*>    def_with_attrs
%type <ASTDecl*>    def_with_ref
%type <ASTDecl*>    def
%type <ASTDecl*>    decl
%type <ASTAttr*>    doc
%type <ASTAttr*>    idoc
%type <ASTNode*>    doc_lit_or_ref
%type <ASTDeclRef*> ref

%type <ASTAttr*> attr_item
%type <ASTAttr*> attr_item_with_args
%type <ASTNode*> arg_item

%type <std::vector<ASTDecl*>> stack_decls
%type <std::vector<ASTNode*>> doc_nodes
%type <std::vector<ASTAttr*>> doc_list
%type <std::vector<ASTAttr*>> attr_list
%type <std::vector<ASTNode*>> arg_list

%start idl

%%

idl : stack_decls { scanner.context().build(); }

stack_decls
    : def_with_attrs { rule(Hierarchy, $1, nullptr); add_symbol($1); $$.push_back($1); }
    | stack_decls def_with_attrs { rule(Hierarchy, $2, $1.back()); add_symbol($2); if ($2->as<ASTImport>()) { $1.push_back($2); } else { $1.back() = $2; } $$ = std::move($1); }
    | stack_decls POPIMPORT { $1.pop_back(); $$ = std::move($1); }
    ;

def_with_attrs
    : def_with_ref { $$ = $1; rule(AttrValidator, $$); }
    | def_with_ref '[' attr_list ']' { $$ = $1; $$->attrs.insert($$->attrs.end(), $3.begin(), $3.end()); rule(AttrValidator, $$); }
    | def_with_ref idoc { $$ = $1; $$->attrs.push_back($2); rule(AttrValidator, $$); }
    | def_with_ref '[' attr_list ']' idoc { $$ = $1; $$->attrs.insert($$->attrs.end(), $3.begin(), $3.end()); $$->attrs.push_back($5); rule(AttrValidator, $$); }
    ;

def_with_ref
    : def { $$ = $1; }
    | def ':' arg_list 
    { 
        $$ = $1; 
        ASTAttr* valOrType = nullptr;
        if (rule(AttrValueOrType, $1).isValue) {
            valOrType = alloc_node(AttrValue, @3); 
        } else {
            valOrType = alloc_node(AttrType, @3); 
        }
        rule(AttrArg, valOrType, $3);
        $$->attrs.push_back(valOrType); 
    }
    ;

def
    : decl ID { $1->name = $2; $$ = $1; }
    | doc_list decl ID { $2->name = $3; $$ = $2; $$->attrs.insert($$->attrs.end(), $1.begin(), $1.end()); }
    ;

doc
    : DOC doc_nodes { $$ = alloc_node(AttrBrief, @2); rule(AttrArg, $$, $2); rule(AttrDocValidator, $$); }
    | DOC doc_nodes '[' attr_item ']' { $$ = $4; rule(AttrArg, $$, $2); rule(AttrDocValidator, $$); }
    ;

idoc
    : IDOC doc_nodes { $$ = alloc_node(AttrDetail, @2); rule(AttrArg, $$, $2); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); }
    | IDOC doc_nodes '[' attr_item ']' { $$ = $4; rule(AttrArg, $$, $2); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); }
    ;

doc_lit_or_ref
    : STR { $$ = add_literal(@1, $1); }
    | '{' ref '}' { $$ = $2; }
    ;

doc_nodes
    : doc_lit_or_ref { $$ = std::vector<ASTNode*>(); $$.push_back($1); }
    | doc_nodes doc_lit_or_ref { $1.push_back($2); $$ = std::move($1); }
    ;

doc_list
    : doc { $$ = std::vector<ASTAttr*>(); $$.push_back($1); }
    | doc_list doc { $1.push_back($2); $$ = std::move($1); }

ref
    : ID { $$ = alloc_node(DeclRef, @1); $$->name = $1; }
    ;

decl
    : API { $$ = alloc_node(Api, @1); }
    | ENUM { $$ = alloc_node(Enum, @1); }
    | CONST { $$ = alloc_node(Const, @1); }
    | IMPORT { $$ = alloc_node(Import, @1); }
    ;

attr_list
    : attr_item_with_args { $$ = std::vector<ASTAttr*>(); $$.push_back($1); }
    | attr_list ',' attr_item_with_args { $1.push_back($3); $$ = std::move($1); }
    ;

attr_item_with_args
    : attr_item { $$ = $1; rule(AttrArg, $$, std::vector<ASTNode*>{}); }
    | attr_item '(' ')' { $$ = $1; rule(AttrArg, $$, std::vector<ASTNode*>{}); log(N1001, @1, visit(AttrName, $$, str)); }
    | attr_item '(' arg_list ')' { $$ = $1; rule(AttrArg, $$, $3); }
    ;

attr_item
    : ATTRVERSION   { $$ = alloc_node(AttrVersion, @1); }
    | ATTRAUTHOR    { $$ = alloc_node(AttrAuthor, @1); }
    | ATTRCOPYRIGHT { $$ = alloc_node(AttrCopyright, @1); }
    | ATTRLICENSE   { $$ = alloc_node(AttrLicense, @1); }
    | ATTRFLAGS     { $$ = alloc_node(AttrFlags, @1); }
    | ATTRVALUE     { $$ = alloc_node(AttrValue, @1); }
    | ATTRTYPE      { $$ = alloc_node(AttrType, @1); }
    | ATTRCNAME     { $$ = alloc_node(AttrCName, @1); }
    | ATTRTOKENIZER { $$ = alloc_node(AttrTokenizer, @1); }
    | ATTRORDER     { $$ = alloc_node(AttrOrder, @1); }
    | ATTRSINGLE    { $$ = alloc_node(AttrSingle, @1); }
    | ATTRHEX       { $$ = alloc_node(AttrHex, @1); }
    | ATTRBRIEF     { $$ = alloc_node(AttrBrief, @1); }
    | ATTRDETAIL    { $$ = alloc_node(AttrDetail, @1); }
    | INVALID_ATTR  { $$ = nullptr; log(E3013, @1, $1); }
    ;

arg_item
    : INT   { $$ = add_literal(@1, $1); }
    | FLOAT { $$ = add_literal(@1, $1); }
    | BOOL  { $$ = add_literal(@1, $1); }
    | STR   { $$ = add_literal(@1, $1); }
    | ref   { $$ = $1; }
    | INVALID_ARG { $$ = nullptr; log(E3002, @1, $1); }
    ;

arg_list
    : arg_item { $$ = std::vector<ASTNode*>(); $$.push_back($1); }
    | arg_list ',' arg_item { $1.push_back($3); $$ = std::move($1); }
    ;

%%

void idl::Parser::error(const location_type& loc, const std::string& message)
{
    log(E3001, loc);
}
