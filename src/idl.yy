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
    #define intern(str) \
        scanner.context().result()->intern({(str).c_str(), (str).length()})
    #define alloc_node(loc, type) \
        scanner.context().result()->allocNode(ASTLocation { \
            intern(*loc.begin.filename), \
            uint16_t(loc.begin.column), \
            uint16_t(loc.begin.line) \
        }, IDL_AST_NODE_TYPE_##type, false)
    #define node(handle) \
        scanner.context().result()->getNode(handle)
    #define node_ref(handle) \
        scanner.context().getNodeRef(handle)
    #define add_child(parent, child) \
        scanner.context().addChild(parent, child)
    #define add_symbol(decl) \
        scanner.context().addSymbol(decl)
    #define log(status, loc, ...) \
        scanner.context().log<IDL_STATUS_##status>({ \
            intern(*loc.begin.filename), \
            uint16_t(loc.begin.column), \
            uint16_t(loc.begin.line) } __VA_OPT__(,) __VA_ARGS__)
    idl::ASTNodeHandleList idl::ASTNodeHandleList::init(ASTNodeHandle node) noexcept { return { node, node, 1u }; }
    idl::ASTNodeHandleList idl::ASTNodeHandleList::add(Scanner& scanner, ASTNodeHandle node) noexcept { 
        if (auto lastNode = scanner.context().result()->getNode(last)) { 
            lastNode->sibling = node; 
            return { first, node, count + 1 };
        }
        return *this;
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
%token STRUCT
%token FIELD
%token FUNC
%token ARG
%token IMPORT
%token POPIMPORT
%token DOC
%token MDOC
%token IDOC
%token IMDOC

%token ATTRVERSION
%token ATTRAUTHOR
%token ATTRCOPYRIGHT
%token ATTRLICENSE
%token ATTRFLAGS
%token ATTRHEX
%token ATTRMAXENUM
%token ATTRCOUNTENUMS
%token ATTRTYPEDENUMS
%token ATTRBRIEF
%token ATTRDETAIL
%token ATTRRETURN
%token ATTRVALUE
%token ATTRTYPE
%token ATTRCNAME
%token ATTRCCONV
%token ATTRCFORMAT
%token ATTRTOKENIZER
%token ATTRARRAY
%token ATTRSINGLE
%token ATTRSTDTYPES
%token ATTRBOOLTYPE
%token ATTRREF
%token ATTRIN
%token ATTROUT
%token ATTRCONST
%token ATTROPTIONAL

%token <std::string> ID
%token <std::string> REF
%token <int64_t>     INT
%token <double>      FLOAT
%token <bool>        BOOL
%token <std::string> STR
%token <std::string> INVALID_ID
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
    if (!scanner.context().result()->hasErrors()) {
        BuildRules::State state {};
        ASTNodeRef api = scanner.context().getNodeRef(scanner.context().result()->getApi());
        api.acceptRecursive<BuildRules>(ASTNodeRef::SkipLiterals | ASTNodeRef::SkipTrivials, std::ref(state));
        state.clearNodes();
    } 
}

stack_decls
    : def_with_attrs { rule(Hierarchy, $1, HandleNone); add_symbol($1); rule(AttrValidator, $1, scanner.context()); $$.push_back($1); }
    | stack_decls def_with_attrs { rule(Hierarchy, $2, $1.back()); add_symbol($2); rule(AttrValidator, $2, scanner.context()); if (isNodeType(node($2), IDL_AST_NODE_TYPE_IMPORT)) { $1.push_back($2); } else { $1.back() = $2; } $$ = std::move($1); }
    | stack_decls POPIMPORT { $1.pop_back(); $$ = std::move($1); }
    ;

def_with_attrs
    : def_with_value { $$ = $1; }
    | def_with_value '[' attr_list ']' { $$ = $1; add_child($$, $3.first); }
    | def_with_value idoc { $$ = $1; add_child($$, $2); }
    | def_with_value '[' attr_list ']' idoc { $$ = $1; add_child($$, $3.first); add_child($$, $5); }
    | def_with_value '[' ']' { $$ = $1; log(N1002, @2); }
    | def_with_value '[' ']' idoc { $$ = $1; add_child($$, $4); log(N1002, @2); }
    ;

def_with_value
    : def_with_type { $$ = $1; }
    | def_with_type ':' arg_list { $$ = $1; auto val = alloc_node(@3, ATTR_VALUE); rule(AttrArg, val, $3.first, $3.last, $3.count); add_child($$, val); }
    ;

def_with_type
    : def { $$ = $1; }
    | def '{' arg_item '}' { $$ = $1; auto type = alloc_node(@3, ATTR_TYPE); rule(AttrArg, type, $3, $3, 1); add_child($$, type);}
    ;

def
    : decl ID { node($1)->name.name = intern($2); $$ = $1; }
    | decl INVALID_ID { node($1)->name.name = intern($2); $$ = $1; log(E3047, @2, $2); }
    | doc_list decl ID { node($2)->name.name = intern($3); $$ = $2; add_child($$, $1.first); }
    | doc_list decl INVALID_ID { node($2)->name.name = intern($3); $$ = $2; add_child($$, $1.first); log(E3047, @3, $3); }
    ;

doc
    : DOC doc_nodes { $$ = alloc_node(@2, ATTR_DOC_BRIEF); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); }
    | DOC doc_nodes '[' ']' { $$ = alloc_node(@2, ATTR_DOC_BRIEF); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); log(N1002, @3); }
    | DOC doc_nodes '[' attr_item ']' { $$ = $4; rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); if (isNodeType(node($4), IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF)) { log(N1003, @4); } }
    | DOC { $$ = alloc_node(@1, ATTR_DOC_BRIEF); rule(AttrArg, $$, HandleNone, HandleNone, 0); rule(AttrDocValidator, $$); }
    | DOC '[' ']' { $$ = alloc_node(@1, ATTR_DOC_BRIEF); rule(AttrArg, $$, HandleNone, HandleNone, 0); rule(AttrDocValidator, $$); log(N1002, @2); }
    | DOC '[' attr_item ']' { $$ = $3; rule(AttrArg, $$, HandleNone, HandleNone, 0); rule(AttrDocValidator, $$); if (isNodeType(node($3), IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF)) { log(N1003, @3); } }
    | MDOC doc_nodes { $$ = alloc_node(@2, ATTR_DOC_BRIEF); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$, true); }
    | MDOC doc_nodes '[' ']' { $$ = alloc_node(@2, ATTR_DOC_BRIEF); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$, true); log(N1002, @3); }
    | MDOC doc_nodes '[' attr_item ']' { $$ = $4; rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$, true); if (isNodeType(node($4), IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF)) { log(N1003, @4); } }
    ;

idoc
    : IDOC doc_nodes { $$ = alloc_node(@2, ATTR_DOC_DETAIL); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); }
    | IDOC doc_nodes '[' ']' { $$ = alloc_node(@2, ATTR_DOC_DETAIL); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); log(N1002, @3); }
    | IDOC doc_nodes '[' attr_item ']' { $$ = $4; rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); if (isNodeType(node($4), IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL)) { log(N1004, @4); } }
    | IDOC { $$ = alloc_node(@1, ATTR_DOC_DETAIL); rule(AttrArg, $$, HandleNone, HandleNone, 0); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); }
    | IDOC '[' ']' { $$ = alloc_node(@1, ATTR_DOC_DETAIL); rule(AttrArg, $$, HandleNone, HandleNone, 0); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); log(N1002, @2); }
    | IDOC '[' attr_item ']' { $$ = $3; rule(AttrArg, $$, HandleNone, HandleNone, 0); rule(AttrDocValidator, $$); rule(AttrIDocValidator, $$); if (isNodeType(node($3), IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL)) { log(N1004, @3); } }
    | IMDOC doc_nodes { $$ = alloc_node(@2, ATTR_DOC_DETAIL); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$, true); rule(AttrIDocValidator, $$); }
    | IMDOC doc_nodes '[' ']' { $$ = alloc_node(@2, ATTR_DOC_DETAIL); rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$, true); rule(AttrIDocValidator, $$); log(N1002, @3); }
    | IMDOC doc_nodes '[' attr_item ']' { $$ = $4; rule(AttrArg, $$, $2.first, $2.last, $2.count); rule(AttrDocValidator, $$, true); rule(AttrIDocValidator, $$); if (isNodeType(node($4), IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL)) { log(N1004, @4); } }
    ;

doc_lit_or_ref
    : STR { $$ = alloc_node(@1, LITERAL_STR); node($$)->valueStr = intern($1); }
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
    : ID { $$ = alloc_node(@1, DECL_REF); node($$)->valueDeclRef.symbol = intern($1); }
    | REF { $$ = alloc_node(@1, DECL_REF); node($$)->valueDeclRef.symbol = intern($1); }
    ;

decl
    : API { $$ = alloc_node(@1, API); }
    | ENUM { $$ = alloc_node(@1, ENUM); }
    | CONST { $$ = alloc_node(@1, CONST); }
    | STRUCT { $$ = alloc_node(@1, STRUCT); }
    | FIELD { $$ = alloc_node(@1, FIELD); }
    | FUNC { $$ = alloc_node(@1, FUNC); }
    | ARG { $$ = alloc_node(@1, ARG); }
    | IMPORT { $$ = alloc_node(@1, IMPORT); }
    ;

attr_list
    : attr_item_with_args { $$ = list_init($1); }
    | attr_list ',' attr_item_with_args { $$ = list_add($1, $3); }
    ;

attr_item_with_args
    : attr_item { $$ = $1; rule(AttrArg, $$, HandleNone, HandleNone, 0); }
    | attr_item '(' ')' { $$ = $1; rule(AttrArg, $$, HandleNone, HandleNone, 0); log(N1001, @1, visit(AttrName, $$, str)); }
    | attr_item '(' arg_list ')' { $$ = $1; rule(AttrArg, $$, $3.first, $3.last, $3.count); }
    ;

attr_item
    : ATTRVERSION    { $$ = alloc_node(@1, ATTR_VERSION); }
    | ATTRAUTHOR     { $$ = alloc_node(@1, ATTR_DOC_AUTHOR); }
    | ATTRCOPYRIGHT  { $$ = alloc_node(@1, ATTR_DOC_COPYRIGHT); }
    | ATTRLICENSE    { $$ = alloc_node(@1, ATTR_DOC_LICENSE); }
    | ATTRFLAGS      { $$ = alloc_node(@1, ATTR_FLAGS); }
    | ATTRVALUE      { $$ = alloc_node(@1, ATTR_VALUE); }
    | ATTRTYPE       { $$ = alloc_node(@1, ATTR_TYPE); }
    | ATTRCNAME      { $$ = alloc_node(@1, ATTR_CNAME); }
    | ATTRCCONV      { $$ = alloc_node(@1, ATTR_CCONV); }
    | ATTRCFORMAT    { $$ = alloc_node(@1, ATTR_CFORMAT); }
    | ATTRTOKENIZER  { $$ = alloc_node(@1, ATTR_TOKENIZER); }
    | ATTRARRAY      { $$ = alloc_node(@1, ATTR_ARRAY); }
    | ATTRSINGLE     { $$ = alloc_node(@1, ATTR_SINGLE); }
    | ATTRSTDTYPES   { $$ = alloc_node(@1, ATTR_STD_TYPES); }
    | ATTRBOOLTYPE   { $$ = alloc_node(@1, ATTR_BOOL_TYPE); }
    | ATTRHEX        { $$ = alloc_node(@1, ATTR_HEX); }
    | ATTRMAXENUM    { $$ = alloc_node(@1, ATTR_MAX_ENUM); }
    | ATTRCOUNTENUMS { $$ = alloc_node(@1, ATTR_COUNT_ENUMS); }
    | ATTRTYPEDENUMS { $$ = alloc_node(@1, ATTR_TYPED_ENUMS); }
    | ATTRREF        { $$ = alloc_node(@1, ATTR_REF); }
    | ATTRIN         { $$ = alloc_node(@1, ATTR_IN); }
    | ATTROUT        { $$ = alloc_node(@1, ATTR_OUT); }
    | ATTRCONST      { $$ = alloc_node(@1, ATTR_CONST); }
    | ATTROPTIONAL   { $$ = alloc_node(@1, ATTR_OPTIONAL); }
    | ATTRBRIEF      { $$ = alloc_node(@1, ATTR_DOC_BRIEF); }
    | ATTRDETAIL     { $$ = alloc_node(@1, ATTR_DOC_DETAIL); }
    | ATTRRETURN     { $$ = alloc_node(@1, ATTR_DOC_RETURN); }
    | INVALID_ATTR   { $$ = HandleNone; log(E3013, @1, $1); }
    ;

arg_item
    : INT   { $$ = alloc_node(@1, LITERAL_INT); node($$)->valueInt = $1; }
    | FLOAT { $$ = alloc_node(@1, LITERAL_FLOAT); node($$)->valueFloat = $1; }
    | BOOL  { $$ = alloc_node(@1, LITERAL_BOOL); node($$)->valueBool = $1; }
    | STR   { $$ = alloc_node(@1, LITERAL_STR); node($$)->valueStr = intern($1); }
    | ref   { $$ = $1; }
    | INVALID_ARG { $$ = HandleNone; log(E3002, @1, $1); }
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
