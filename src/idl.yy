%require "3.7.4"
%language "C++"
%defines "parser.hpp"
%output "parser.cpp"
%locations

%{
#include <map>
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
    #define alloc_node(ast, loc, token) \
        scanner.context().allocNode<ast, idl::Parser::syntax_error>(loc, token)
    #define intern(loc, str) \
        scanner.context().intern<idl::Parser::syntax_error>(loc, str, idl::Parser::token::STR)
    #define intern_bool(loc, b) \
        scanner.context().intern<idl::Parser::syntax_error>(loc, b, idl::Parser::token::BOOL)
    #define intern_int(loc, num) \
        scanner.context().intern<idl::Parser::syntax_error>(loc, num, idl::Parser::token::NUM)
    #define last_enum(loc) \
        scanner.context().lastEnum<idl::Parser::syntax_error>(loc)
    
    void addNode(idl::Context&, ASTDecl*, ASTDecl*);
    void addDoc(ASTDoc*, const std::vector<ASTNode*>&, char);
    void addAttrs(ASTDecl*, const std::vector<ASTAttr*>&);
    void addAttrPlatformArgs(ASTAttr*, const std::vector<std::string>&);
    void addAttrValueArgs(idl::Scanner&, ASTAttr*, ASTLiteral);
    void addAttrTypeArgs(idl::Scanner&, ASTAttr*, const std::vector<std::string>&);
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

%token ATTRFLAGS
%token ATTRHEX
%token ATTRPLATFORM
%token ATTRVALUE
%token ATTRTYPE
%token <std::string> ATTRARG

%token API
%token ENUM
%token CONST
%token STRUCT
%token FIELD

%token <std::string> STR
%token <std::string> ID
%token <int64_t> NUM
%token <bool> BOOL

%type <ASTAttr*> attr_item
%type <std::vector<ASTAttr*>> attr_list
%type <std::vector<std::string>> attr_args
%type <std::vector<std::string>> attr_arg_list

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
    : def_with_attrs { throw syntax_error(@1, err_str<E2005>()); }
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
    | def '[' attr_list ']' { addAttrs($1, $3); $$ = $1; }
    ;

def
    : def_with_type { $$ = $1; }
    | def_with_type ':' NUM { 
        ASTAttr::Arg arg{};
        arg.value = intern_int(@3, $3);
        auto attr = alloc_node(ASTAttr, @3, token::ATTRVALUE);
        attr->type = ASTAttr::Value;
        attr->args.push_back(arg);
        addAttrs($1, { attr });
        $$ = $1;
    }
    | def_with_type ':' BOOL { 
        ASTAttr::Arg arg{};
        arg.value = intern_bool(@3, $3);
        auto attr = alloc_node(ASTAttr, @3, token::ATTRVALUE);
        attr->type = ASTAttr::Value;
        attr->args.push_back(arg);
        addAttrs($1, { attr });
        $$ = $1;
    }
    ;

def_with_type
    : decl ID { $1->name = $2; $$ = $1; }
    | decl ID '{' ID '}' {
        $1->name = $2;
        auto attr = alloc_node(ASTAttr, @4, token::ATTRTYPE);
        attr->type = ASTAttr::Type;
        addAttrTypeArgs(scanner, attr, { $4 });
        addAttrs($1, { attr });
        $$ = $1;
    }
    ;

decl
    : API { auto node = alloc_node(ASTApi, @1, token::API); $$ = node; }
    | ENUM { auto node = alloc_node(ASTEnum, @1, token::ENUM); $$ = node; }
    | CONST { auto node = alloc_node(ASTEnumConst, @1, token::CONST); $$ = node; }
    | STRUCT { auto node = alloc_node(ASTStruct, @1, token::STRUCT); $$ = node; }
    | FIELD { auto node = alloc_node(ASTField, @1, token::FIELD); $$ = node; }
    ;

attr_list
    : attr_item { auto list = std::vector<ASTAttr*>(); list.push_back($1); $$ = list; }
    | attr_list ',' attr_item { $1.push_back($3); $$ = $1; }
    ;

attr_item
    : ATTRFLAGS { auto node = alloc_node(ASTAttr, @1, token::ATTRFLAGS); node->type = ASTAttr::Flags; $$ = node; }
    | ATTRHEX { auto node = alloc_node(ASTAttr, @1, token::ATTRHEX); node->type = ASTAttr::Hex; $$ = node; }
    | ATTRPLATFORM { throw syntax_error(@1, err_str<E2016>()); }
    | ATTRPLATFORM attr_args {
        auto node = alloc_node(ASTAttr, @1, token::ATTRPLATFORM);
        node->type = ASTAttr::Platform;
        addAttrPlatformArgs(node, $2);
        $$ = node;
    }
    | ATTRVALUE { throw syntax_error(@1, err_str<E2023>()); }
    | ATTRVALUE '(' NUM ')' {
        ASTAttr::Arg arg{};
        arg.value = intern_int(@3, $3);
        auto node = alloc_node(ASTAttr, @1, token::ATTRVALUE);
        node->type = ASTAttr::Value;
        node->args.push_back(arg);
        $$ = node;
    }
    | ATTRVALUE '(' BOOL ')' {
        ASTAttr::Arg arg{};
        arg.value = intern_bool(@3, $3);
        auto node = alloc_node(ASTAttr, @1, token::ATTRVALUE);
        node->type = ASTAttr::Value;
        node->args.push_back(arg);
        $$ = node;
    }
    | ATTRVALUE attr_args {
        auto node = alloc_node(ASTAttr, @1, token::ATTRVALUE);
        node->type = ASTAttr::Value;
        for (const auto& name : $2) {
            auto lit = alloc_node(ASTLiteralEnumConst, @1, token::NUM);
            lit->name = name;
            ASTAttr::Arg arg{};
            arg.value = lit;
            node->args.push_back(arg);
        }
        $$ = node;
    }
    | ATTRTYPE { throw syntax_error(@1, err_str<E2028>()); }
    | ATTRTYPE attr_args {
        auto node = alloc_node(ASTAttr, @1, token::ATTRTYPE);
        node->type = ASTAttr::Type;
        addAttrTypeArgs(scanner, node, $2);
        $$ = node;
    }
    ;

attr_args
    : '(' attr_arg_list ')' { $$ = $2; }
    ;

attr_arg_list
    : ID { auto list = std::vector<std::string>(); list.push_back($1); $$ = list; }
    | ATTRARG { auto list = std::vector<std::string>(); list.push_back($1); $$ = list; }
    | attr_arg_list ',' ATTRARG { $1.push_back($3); $$ = $1; }
    | attr_arg_list ',' ID { $1.push_back($3); $$ = $1; }
    ;

doc
    : doc_decl { auto node = alloc_node(ASTDoc, @1, token::DOC); addDoc(node, $1.first, $1.second); $$ = node; }
    | doc doc_decl { addDoc($1, $2.first, $2.second); $$ = $1; }
    ;

idoc
    : idoc_decl { auto node = alloc_node(ASTDoc, @1, token::IDOC); addDoc(node, $1.first, $1.second); $$ = node; }
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
    | '{' STR '}' { auto node = alloc_node(ASTDeclRef, @2, token::STR); node->name = $2; $$ = node; }
    ;

%%

void addNode(idl::Context& context, ASTDecl* prev, ASTDecl* decl)
{
    if (prev == nullptr)
    {
        if (decl->token != idl::Parser::token::API)
        {
            throw idl::Parser::syntax_error(decl->location, err_str<E2012>());
        }
        context.initBuiltins<idl::Parser::syntax_error>();
        return;
    }
    if (decl->token == idl::Parser::token::API)
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
    }
}

void addAttrs(ASTDecl* node, const std::vector<ASTAttr*>& attrs)
{
    node->attrs.insert(node->attrs.end(), attrs.begin(), attrs.end());
    std::sort(node->attrs.begin(), node->attrs.end(), [](auto attr1, auto attr2)
    {
        return attr1->type < attr2->type;
    });
    auto it = std::unique(node->attrs.begin(), node->attrs.end(), [](auto attr1, auto attr2)
    {
        return attr1->type == attr2->type;
    });
    if (it != node->attrs.end())
    {
        throw idl::Parser::syntax_error((*it)->location, err_str<E2013>((*it)->typeStr()));
    }
    AllowedAttrs allowAttrs{};
    node->accept(allowAttrs);
    for (auto attr : node->attrs)
    {
        if (!allowAttrs.allowed.contains(attr->type))
        {
            auto first = true;
            std::ostringstream ss;
            for (auto type : allowAttrs.allowed) {
                if (!first) {
                    ss << ", ";
                }
                ss << '\'' << ASTAttr::typeStr(type) << '\'';
                first = false;
            }
            throw idl::Parser::syntax_error(attr->location, err_str<E2014>(ss.str()));
        }
        attr->parent = node;
    }
}

void addAttrPlatformArgs(ASTAttr* node, const std::vector<std::string>& platforms)
{
    std::map<std::string, TargetPlatfrom> allowedPlatforms;
    for (const auto& platform : magic_enum::enum_values<TargetPlatfrom>()) {
        std::string str = magic_enum::enum_name(platform).data();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        allowedPlatforms[str] = platform;
    }
    for (const auto& arg : platforms) {
        auto it = allowedPlatforms.find(arg);
        if (it == allowedPlatforms.end()) {
            auto first = true;
            std::ostringstream ss;
            for (const auto& [name, _] : allowedPlatforms) {
                if (!first) {
                    ss << ", ";
                }
                ss << '\'' << name << '\'';
                first = false;
            }
            throw idl::Parser::syntax_error(node->location, err_str<E2017>(ss.str()));
        }
        ASTAttr::Arg argPlatform{};
        argPlatform.platform = it->second;
        node->args.push_back(argPlatform);
    }
    std::sort(node->args.begin(), node->args.end(), [](auto p1, auto p2) {
        return (int) p1.platform < (int) p2.platform;
    });
    auto it = std::unique(node->args.begin(), node->args.end(), [](auto p1, auto p2) {
        return p1.platform == p2.platform;
    });
    if (it != node->args.end())
    {
        std::string str = magic_enum::enum_name(it->platform).data();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        throw idl::Parser::syntax_error(node->location, err_str<E2018>(str));
    }
}

void addAttrTypeArgs(idl::Scanner& scanner, ASTAttr* node, const std::vector<std::string>& types)
{
    if (types.size() != 1) 
    {
        throw idl::Parser::syntax_error(node->location, err_str<E2029>());
    }
    const auto& str = types[0];
    ASTAttr::Arg arg{};
    arg.type = alloc_node(ASTDeclRef, node->location, idl::Parser::token::STR);
    arg.type->name = str;
    arg.type->parent = node;
    node->args.push_back(arg);
}

void idl::Parser::error(const location_type& loc, const std::string& message)
{
   std::cerr << "error: " << message << " at " << Location(loc) << std::endl;
}
