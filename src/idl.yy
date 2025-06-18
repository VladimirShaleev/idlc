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
#include "visitors.hpp"
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
    #define add_attrs(node, attrs) \
        scanner.context().addAttrs<idl::Parser::syntax_error>(node, attrs)
    #define intern(loc, str) \
        scanner.context().intern<idl::Parser::syntax_error>(loc, str)
    #define intern_bool(loc, b) \
        scanner.context().intern<idl::Parser::syntax_error>(loc, b)
    #define intern_int(loc, num) \
        scanner.context().intern<idl::Parser::syntax_error>(loc, num)
    
    void addNode(idl::Context&, ASTDecl*, ASTDecl*);
    void addDoc(ASTDoc*, const std::vector<ASTNode*>&, char);
}

%initial-action {
    @$.begin.filename = @$.end.filename = scanner.filename();
}

%token IDOC
%token DOC
%token DOCBRIEF
%token DOCDETAIL
%token DOCCOPYRIGHT
%token DOCLICENSE
%token DOCAUTHOR
%token DOCSEE
%token DOCNOTE
%token DOCWARN
%token DOCRETURN

%token ATTRFLAGS
%token ATTRHEX
%token ATTRPLATFORM
%token ATTRVALUE
%token ATTRTYPE
%token ATTRSTATIC
%token ATTRCTOR
%token ATTRTHIS
%token ATTRGET
%token ATTRSET
%token ATTRHANDLE
%token ATTRCNAME
%token ATTRARRAY
%token ATTRCONST
%token ATTRREF

%token API
%token ENUM
%token CONST
%token STRUCT
%token FIELD
%token INTERFACE
%token METHOD
%token ARG
%token PROP
%token HANDLE
%token FUNC

%token <std::string> STR
%token <std::string> ID
%token <std::string> REF
%token <int64_t> NUM
%token <bool> BOOL
%token <ASTAttrPlatform::Type> ATTRPLATFORMARG

%type <ASTAttr*> attr_item
%type <ASTAttr*> attr_flags
%type <ASTAttr*> attr_hex
%type <ASTAttr*> attr_platform
%type <ASTAttr*> attr_type
%type <ASTAttr*> attr_cname
%type <ASTAttr*> attr_array
%type <ASTAttr*> attr_const
%type <ASTAttr*> attr_ref
%type <ASTAttr*> attr_get
%type <ASTAttr*> attr_set
%type <ASTAttr*> attr_handle
%type <ASTAttr*> attr_value
%type <ASTAttr*> attr_static
%type <ASTAttr*> attr_ctor
%type <ASTAttr*> attr_this
%type <std::vector<ASTDeclRef*>> attr_ref_arg_list
%type <ASTAttrPlatform::Type> attr_platform_arg_list
%type <std::vector<ASTAttr*>> attr_list

%type <ASTDoc*> doc
%type <ASTDoc*> idoc
%type <ASTNode*> doc_lit_or_ref
%type <std::vector<ASTNode*>> doc_field
%type <std::pair<std::vector<ASTNode*>, char>> doc_decl
%type <std::pair<std::vector<ASTNode*>, char>> idoc_decl

%type <ASTDecl*> node
%type <ASTDecl*> def_with_attrs_and_doc
%type <ASTDecl*> def_with_attrs
%type <ASTDecl*> def
%type <ASTDecl*> def_with_type
%type <ASTDecl*> decl

%start node

%%

node
    : def_with_attrs_and_doc { addNode(scanner.context(), nullptr, $1); $$ = $1; }
    | node def_with_attrs_and_doc { addNode(scanner.context(), $1, $2); $$ = $2; }
    ;

def_with_attrs_and_doc
    : def_with_attrs { throw syntax_error(@1, err_str<E2005>($1->fullname())); }
    | doc def_with_attrs idoc { throw syntax_error(@2, err_str<E2021>()); }
    | doc def_with_attrs {
        $2->doc = $1;
        $1->parent = $2;
        $$ = $2;
    }
    | def_with_attrs idoc {
        $1->doc = $2;
        $2->parent = $1;
        $$ = $1;
    }
    ;

def_with_attrs
    : def { $$ = $1; }
    | def '[' attr_list ']' { add_attrs($1, $3); $$ = $1; }
    ;

def
    : def_with_type { $$ = $1; }
    | def_with_type ':' NUM { 
        auto attr = alloc_node(ASTAttrValue, @1);
        attr->value = intern_int(@3, $3);
        add_attrs($1, { attr });
        $$ = $1;
    }
    | def_with_type ':' BOOL { 
        auto attr = alloc_node(ASTAttrValue, @1);
        attr->value = intern_bool(@3, $3);
        add_attrs($1, { attr });
        $$ = $1;
    }
    | def_with_type ':' attr_ref_arg_list { 
        auto consts = alloc_node(ASTLiteralConsts, @3);
        consts->decls = $3;
        auto attr = alloc_node(ASTAttrValue, @1);
        attr->value = consts;
        for (auto decl : consts->decls) {
            decl->parent = $1;
        }
        add_attrs($1, { attr });
        $$ = $1;
    }
    ;

def_with_type
    : decl ID { $1->name = $2; $$ = $1; }
    | decl ID '{' REF '}' {
        auto ref = alloc_node(ASTDeclRef, @4);
        ref->name = $4;
        auto attr = alloc_node(ASTAttrType, @4);
        attr->type = ref;
        ref->parent = attr;
        $1->name = $2;
        add_attrs($1, { attr });
        $$ = $1;
    }
    ;

decl
    : API { auto node = alloc_node(ASTApi, @1); $$ = node; }
    | ENUM { auto node = alloc_node(ASTEnum, @1); $$ = node; }
    | CONST { auto node = alloc_node(ASTEnumConst, @1); $$ = node; }
    | STRUCT { auto node = alloc_node(ASTStruct, @1); $$ = node; }
    | FIELD { auto node = alloc_node(ASTField, @1); $$ = node; }
    | INTERFACE { auto node = alloc_node(ASTInterface, @1); $$ = node; }
    | METHOD { auto node = alloc_node(ASTMethod, @1); $$ = node; }
    | ARG { auto node = alloc_node(ASTArg, @1); $$ = node; }
    | PROP { auto node = alloc_node(ASTProperty, @1); $$ = node; }
    | HANDLE { auto node = alloc_node(ASTHandle, @1); $$ = node; }
    | FUNC { auto node = alloc_node(ASTFunc, @1); $$ = node; }
    ;

attr_list
    : attr_item { auto list = std::vector<ASTAttr*>(); list.push_back($1); $$ = list; }
    | attr_list ',' attr_item { $1.push_back($3); $$ = $1; }
    ;

attr_item
    : attr_flags { $$ = $1; }
    | attr_hex { $$ = $1; }
    | attr_platform { $$ = $1; }
    | attr_value { $$ = $1; }
    | attr_type { $$ = $1; }
    | attr_cname { $$ = $1; }
    | attr_array { $$ = $1; }
    | attr_const { $$ = $1; }
    | attr_ref { $$ = $1; }
    | attr_get { $$ = $1; }
    | attr_set { $$ = $1; }
    | attr_handle { $$ = $1; }
    | attr_static { $$ = $1; }
    | attr_ctor { $$ = $1; }
    | attr_this { $$ = $1; }
    ;

attr_flags
    : ATTRFLAGS { auto node = alloc_node(ASTAttrFlags, @1); $$ = node; }
    ;

attr_hex
    : ATTRHEX { auto node = alloc_node(ASTAttrHex, @1); $$ = node; }
    ;

attr_platform
    : ATTRPLATFORM { throw syntax_error(@1, err_str<E2016>()); }
    | ATTRPLATFORM '(' ')' { throw syntax_error(@1, err_str<E2016>()); }
    | ATTRPLATFORM '(' attr_platform_arg_list ')' {
        auto node = alloc_node(ASTAttrPlatform, @1);
        node->platforms = $3;
        $$ = node;
    }
    ;

attr_value
    : ATTRVALUE { throw syntax_error(@1, err_str<E2023>()); }
    | ATTRVALUE '(' ')' { throw syntax_error(@1, err_str<E2023>()); }
    | ATTRVALUE '(' NUM ')' {
        auto node = alloc_node(ASTAttrValue, @1);
        node->value = intern_int(@3, $3);
        $$ = node;
    }
    | ATTRVALUE '(' BOOL ')' {
        auto node = alloc_node(ASTAttrValue, @1);
        node->value = intern_bool(@3, $3);
        $$ = node;
    }
    | ATTRVALUE '(' attr_ref_arg_list ')' {
        auto consts = alloc_node(ASTLiteralConsts, @3);
        consts->decls = $attr_ref_arg_list;
        auto node = alloc_node(ASTAttrValue, @1);
        node->value = consts;
        for (auto decl : consts->decls) {
            decl->parent = node;
        }
        $$ = node;
    }
    ;

attr_type
    : ATTRTYPE { throw syntax_error(@1, err_str<E2049>()); }
    | ATTRTYPE '(' ')' { throw syntax_error(@1, err_str<E2049>()); }
    | ATTRTYPE '(' REF ')' {
        auto ref = alloc_node(ASTDeclRef, @3);
        ref->name = $3;
        auto node = alloc_node(ASTAttrType, @1);
        node->type = ref;
        ref->parent = node;
        $$ = node;
    }
    ;

attr_cname
    : ATTRCNAME { throw syntax_error(@1, err_str<E2075>()); }
    | ATTRCNAME '(' ')' { throw syntax_error(@1, err_str<E2075>()); }
    | ATTRCNAME '(' STR ')' {
        auto node = alloc_node(ASTAttrCName, @1);
        node->name = $3;
        $$ = node;
    }
    ;

attr_array
    : ATTRARRAY { throw syntax_error(@1, err_str<E2076>()); }
    | ATTRARRAY '(' ')' { throw syntax_error(@1, err_str<E2076>()); }
    | ATTRARRAY '(' NUM ')' {
        auto node = alloc_node(ASTAttrArray, @1);
        node->size = $3;
        $$ = node;
    }
    | ATTRARRAY '(' REF ')' {
        auto ref = alloc_node(ASTDeclRef, @3);
        ref->name = $3;
        auto node = alloc_node(ASTAttrArray, @1);
        node->ref = true;
        node->decl = ref;
        ref->parent = node;
        $$ = node;
    }
    ;

attr_const
    : ATTRCONST { auto node = alloc_node(ASTAttrConst, @1); $$ = node; }
    ;

attr_ref
    : ATTRREF { auto node = alloc_node(ASTAttrRef, @1); $$ = node; }
    ;

attr_get
    : ATTRGET { throw syntax_error(@1, err_str<E2050>()); }
    | ATTRGET '(' ')' { throw syntax_error(@1, err_str<E2050>()); }
    | ATTRGET '(' REF ')' {
        auto ref = alloc_node(ASTDeclRef, @3);
        ref->name = $3;
        auto node = alloc_node(ASTAttrGet, @1);
        node->decl = ref;
        ref->parent = node;
        $$ = node;
    }
    ;

attr_set
    : ATTRSET { throw syntax_error(@1, err_str<E2050>()); }
    | ATTRSET '(' ')' { throw syntax_error(@1, err_str<E2050>()); }
    | ATTRSET '(' REF ')' {
        auto ref = alloc_node(ASTDeclRef, @3);
        ref->name = $3;
        auto node = alloc_node(ASTAttrSet, @1);
        node->decl = ref;
        ref->parent = node;
        $$ = node;
    }
    ;

attr_handle
    : ATTRHANDLE { auto node = alloc_node(ASTAttrHandle, @1); $$ = node; }
    ;

attr_static
    : ATTRSTATIC { auto node = alloc_node(ASTAttrStatic, @1); $$ = node; }
    ;

attr_ctor
    : ATTRCTOR { auto node = alloc_node(ASTAttrCtor, @1); $$ = node; }
    ;

attr_this
    : ATTRTHIS { auto node = alloc_node(ASTAttrThis, @1); $$ = node; }
    ;

attr_ref_arg_list
    : REF { 
        auto node = alloc_node(ASTDeclRef, @1);
        node->name = $1;
        auto list = std::vector<ASTDeclRef*>();
        list.push_back(node);
        $$ = list;
    }
    | attr_ref_arg_list ',' REF {
        auto node = alloc_node(ASTDeclRef, @3);
        node->name = $3;
        $1.push_back(node);
        $$ = $1;
    }
    ;

attr_platform_arg_list  
    : ATTRPLATFORMARG { $$ = $1; }
    | attr_platform_arg_list ',' ATTRPLATFORMARG { 
        if ($1 & $3) {
            std::string str = magic_enum::enum_name($3).data();
            std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
                return std::tolower(c);
            });
            throw syntax_error(@3, err_str<E2018>(str));
        }
        $$ = ASTAttrPlatform::Type($1 | $3);
    }
    ;

doc
    : doc_decl { auto node = alloc_node(ASTDoc, @1); addDoc(node, $1.first, $1.second); $$ = node; }
    | doc doc_decl { addDoc($1, $2.first, $2.second); $$ = $1; }
    ;

idoc
    : idoc_decl { auto node = alloc_node(ASTDoc, @1); addDoc(node, $1.first, $1.second); $$ = node; }
    | idoc idoc_decl { addDoc($1, $2.first, $2.second); $$ = $1; }
    ;

doc_decl
    : DOC { throw syntax_error(@$, err_str<E2006>()); }
    | DOC doc_field              { $$ = std::make_pair($2, 'b'); }
    | DOC doc_field DOCBRIEF     { $$ = std::make_pair($2, 'b'); }
    | DOC doc_field DOCDETAIL    { $$ = std::make_pair($2, 'd'); }
    | DOC doc_field DOCCOPYRIGHT { $$ = std::make_pair($2, 'c'); }
    | DOC doc_field DOCLICENSE   { $$ = std::make_pair($2, 'l'); }
    | DOC doc_field DOCAUTHOR    { $$ = std::make_pair($2, 'a'); }
    | DOC doc_field DOCSEE       { $$ = std::make_pair($2, 's'); }
    | DOC doc_field DOCNOTE      { $$ = std::make_pair($2, 'n'); }
    | DOC doc_field DOCWARN      { $$ = std::make_pair($2, 'w'); }
    | DOC doc_field DOCRETURN    { $$ = std::make_pair($2, 'r'); }
    ;

idoc_decl
    : IDOC { throw syntax_error(@$, err_str<E2006>()); }
    | IDOC doc_field              { $$ = std::make_pair($2, 'd'); }
    | IDOC doc_field DOCDETAIL    { $$ = std::make_pair($2, 'd'); }
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

void addNode(idl::Context& context, ASTDecl* prev, ASTDecl* decl)
{
    if (prev == nullptr)
    {
        if (dynamic_cast<ASTApi*>(decl) == nullptr)
        {
            throw idl::Parser::syntax_error(decl->location, err_str<E2012>());
        }
        context.initBuiltins<idl::Parser::syntax_error>();
        return;
    }
    if (dynamic_cast<ASTApi*>(decl) != nullptr)
    {
        throw idl::Parser::syntax_error(decl->location, err_str<E2004>());
    }
    
    ChildsAggregator<idl::Parser::syntax_error> aggregator = prev;
    decl->accept(aggregator);
    context.addSymbol<idl::Parser::syntax_error>(decl);
}

void addDoc(ASTDoc* node, const std::vector<ASTNode*>& doc, char type)
{
    switch (type)
    {
        case 'b':
            if (!node->brief.empty()) throw idl::Parser::syntax_error(node->location, err_str<E2007>());
            node->brief = doc;
            break;
        case 'd':
            if (!node->detail.empty()) throw idl::Parser::syntax_error(node->location, err_str<E2008>());
            node->detail = doc;
            break;
        case 'c':
            if (!node->copyright.empty()) throw idl::Parser::syntax_error(node->location, err_str<E2009>());
            node->copyright = doc;
            break;
        case 'l':
            if (!node->license.empty()) throw idl::Parser::syntax_error(node->location, err_str<E2010>());
            node->license = doc;
            break;
        case 'a':
            node->authors.push_back(doc);
            break;
        case 's':
            node->see.push_back(doc);
            break;
        case 'n':
            node->note.push_back(doc);
            break;
        case 'w':
            node->warn.push_back(doc);
            break;
        case 'r':
            node->ret = doc;
            break;
    }
}

void idl::Parser::error(const location_type& loc, const std::string& message)
{
   std::cerr << "error: " << message << " at " << Location(loc) << std::endl;
}
