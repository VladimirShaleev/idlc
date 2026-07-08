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
        struct ASTNodeHandleList {
            ASTNodeHandle first;
            ASTNodeHandle last;
            size_t count;
            static ASTNodeHandleList init(ASTNodeHandle node) noexcept;
            ASTNodeHandleList add(Scanner& scanner, ASTNodeHandle node) noexcept;
        };
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
    #define alloc_node(loc, type) \
        scanner.context().allocNode(loc, ASTNodeType::type)
    #define node(handle) \
        scanner.context().getNode(handle)
    #define add_child(parent, child) \
        scanner.context().addChild(parent, child)
    #define intern(str) \
        scanner.context().intern({(str).c_str(), (str).length()})
    #define add_symbol(decl) \
        scanner.context().addSymbol(decl)
    #define log(status, loc, ...) \
        scanner.context().log<IDL_STATUS_##status>({ \
            intern(*loc.begin.filename), \
            uint16_t(loc.begin.line), \
            uint16_t(loc.begin.column) } __VA_OPT__(,) __VA_ARGS__)
    idl::ASTNodeHandleList idl::ASTNodeHandleList::init(ASTNodeHandle node) noexcept { return { node, node, 1u }; }
    idl::ASTNodeHandleList idl::ASTNodeHandleList::add(Scanner& scanner, ASTNodeHandle node) noexcept { 
        scanner.context().getNode(last)->sibling = node; return { first, node, count + 1 };
    }
    #define list_init(node) ASTNodeHandleList::init(node)
    #define list_add(list, node) list.add(scanner, node)
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
%token <std::string> REF
%token <int64_t>     INT
%token <double>      FLOAT
%token <bool>        BOOL
%token <std::string> STR
%token <std::string> INVALID_ARG
%token <std::string> INVALID_ATTR

%type <ASTNodeHandle> def_with_attrs
%type <ASTNodeHandle> def_with_type
%type <ASTNodeHandle> def_with_value
%type <ASTNodeHandle> def
%type <ASTNodeHandle> decl
%type <ASTNodeHandle> doc
%type <ASTNodeHandle> idoc
%type <ASTNodeHandle> doc_lit_or_ref
%type <ASTNodeHandle> ref

%type <ASTNodeHandle> attr_item
%type <ASTNodeHandle> attr_item_with_args
%type <ASTNodeHandle> arg_item

%type <ASTNodeHandleList> doc_nodes
%type <ASTNodeHandleList> doc_list
%type <ASTNodeHandleList> attr_list
%type <ASTNodeHandleList> arg_list

%type <std::vector<ASTNodeHandle>> stack_decls

%start idl

%%

idl : stack_decls { 
    if (!scanner.context().hasErrors()) {
        BuildRules::State state { ASTNodeRef(scanner.context()) };
        scanner.context().api().acceptRecursive<BuildRules>(ASTNodeRef::SkipLiterals | ASTNodeRef::SkipTrivials, std::ref(state));
    } 
}

stack_decls
    : def_with_attrs { rule(Hierarchy, $1, NodeHandleNone); add_symbol($1); $$.push_back($1); }
    | stack_decls def_with_attrs { rule(Hierarchy, $2, $1.back()); add_symbol($2); if (astNodeIs(node($2), ASTNodeType::Import)) { $1.push_back($2); } else { $1.back() = $2; } $$ = std::move($1); }
    | stack_decls POPIMPORT { $1.pop_back(); $$ = std::move($1); }
    ;

def_with_attrs
    : def_with_type { $$ = $1; rule(AttrValidator, $$, scanner.context()); }
    | def_with_type '[' attr_list ']' { $$ = $1; add_child($$, $3.first); rule(AttrValidator, $$, scanner.context()); }
    | def_with_type idoc { $$ = $1; add_child($$, $2); rule(AttrValidator, $$, scanner.context()); }
    | def_with_type '[' attr_list ']' idoc { $$ = $1; add_child($$, $3.first); add_child($$, $5); rule(AttrValidator, $$, scanner.context()); }
    ;

def_with_type
    : def_with_value { $$ = $1; }
    | def_with_value '{' arg_item '}' { $$ = $1; auto type = alloc_node(@3, AttrType); rule(AttrArg, type, $3, $3, 1); add_child($$, type);}
    ;

def_with_value
    : def { $$ = $1; }
    | def ':' arg_list { $$ = $1; auto val = alloc_node(@3, AttrValue); rule(AttrArg, val, $3.first, $3.last, $3.count); add_child($$, val); }
    ;

def
    : decl ID { node($1)->valueStr = intern($2); $$ = $1; }
    | doc_list decl ID { node($2)->valueStr = intern($3); $$ = $2; add_child($$, $1.first); }
    ;

doc
    : DOC doc_nodes { $$ = alloc_node(@2, AttrDocBrief); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); }
    | DOC doc_nodes '[' attr_item ']' { $$ = $4; rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); }
    ;

idoc
    : IDOC doc_nodes { $$ = alloc_node(@2, AttrDocDetail); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); }
    | IDOC doc_nodes '[' attr_item ']' { $$ = $4; rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); }
    ;

doc_lit_or_ref
    : STR { $$ = alloc_node(@1, LiteralStr); node($$)->valueStr = intern($1); }
    | '{' ref '}' { $$ = $2; }
    ;

doc_nodes
    : doc_lit_or_ref { $$ = list_init($1); }
    | doc_nodes doc_lit_or_ref { $$ = list_add($1, $2); }
    ;

doc_list
    : doc { $$ = list_init($1); }
    | doc_list doc { $$ = list_add($1, $2); }

ref
    : ID { $$ = alloc_node(@1, DeclRef); node($$)->valueStr = intern($1); }
    | REF { $$ = alloc_node(@1, DeclRef); node($$)->valueStr = intern($1); }
    ;

decl
    : API { $$ = alloc_node(@1, Api); }
    | ENUM { $$ = alloc_node(@1, Enum); }
    | CONST { $$ = alloc_node(@1, Const); }
    | IMPORT { $$ = alloc_node(@1, Import); }
    ;

attr_list
    : attr_item_with_args { $$ = list_init($1); }
    | attr_list ',' attr_item_with_args { $$ = list_add($1, $3); }
    ;

attr_item_with_args
    : attr_item { $$ = $1; rule(AttrArg, $$, NodeHandleNone, NodeHandleNone, 0); }
    | attr_item '(' ')' { $$ = $1; rule(AttrArg, $$, NodeHandleNone, NodeHandleNone, 0); log(N1001, @1, visit(AttrName, $$, str)); }
    | attr_item '(' arg_list ')' { $$ = $1; rule(AttrArg, $$, $3.first, $3.last, $3.count); }
    ;

attr_item
    : ATTRVERSION   { $$ = alloc_node(@1, AttrVersion); }
    | ATTRAUTHOR    { $$ = alloc_node(@1, AttrDocAuthor); }
    | ATTRCOPYRIGHT { $$ = alloc_node(@1, AttrDocCopyright); }
    | ATTRLICENSE   { $$ = alloc_node(@1, AttrDocLicense); }
    | ATTRFLAGS     { $$ = alloc_node(@1, AttrFlags); }
    | ATTRVALUE     { $$ = alloc_node(@1, AttrValue); }
    | ATTRTYPE      { $$ = alloc_node(@1, AttrType); }
    | ATTRCNAME     { $$ = alloc_node(@1, AttrCName); }
    | ATTRTOKENIZER { $$ = alloc_node(@1, AttrTokenizer); }
    | ATTRORDER     { $$ = alloc_node(@1, AttrOrder); }
    | ATTRSINGLE    { $$ = alloc_node(@1, AttrSingle); }
    | ATTRHEX       { $$ = alloc_node(@1, AttrHex); }
    | ATTRBRIEF     { $$ = alloc_node(@1, AttrDocBrief); }
    | ATTRDETAIL    { $$ = alloc_node(@1, AttrDocDetail); }
    | INVALID_ATTR  { $$ = NodeHandleNone; log(E3013, @1, $1); }
    ;

arg_item
    : INT   { $$ = alloc_node(@1, LiteralInt); node($$)->valueInt = $1; }
    | FLOAT { $$ = alloc_node(@1, LiteralFloat); node($$)->valueFloat = $1; }
    | BOOL  { $$ = alloc_node(@1, LiteralBool); node($$)->valueBool = $1; }
    | STR   { $$ = alloc_node(@1, LiteralStr); node($$)->valueStr = intern($1); }
    | ref   { $$ = $1; }
    | INVALID_ARG { $$ = NodeHandleNone; log(E3002, @1, $1); }
    ;

arg_list
    : arg_item { $$ = list_init($1); }
    | arg_list ',' arg_item { $$ = list_add($1, $3); }
    ;

%%

void idl::Parser::error(const location_type& loc, const std::string& message)
{
    log(E3001, loc);
}
