%require "3.7.4"
%language "C++"
%defines "parser.hpp"
%output "parser.cpp"
%locations

%{
#include <set>
#include <map>
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
    #define last_enum(loc) \
        scanner.context().lastEnum<idl::Parser::syntax_error>(loc)
    
    void addDoc(ASTDoc*, const std::vector<ASTNode*>&, char);
    void addAttrs(ASTDecl*, const std::vector<ASTAttr*>&, const std::set<ASTAttr::Type>&);
    void addAttrPlatformArgs(ASTAttr*, const std::vector<std::string>&);
    void addAttrValueArgs(ASTAttr*, const std::vector<std::string>&);
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
%token <std::string> ATTRARG

%token API
%token ENUM
%token CONST

%token <std::string> STR
%token <std::string> ID
%token <int64_t> NUM

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

%type <ASTApi*> root
%type <ASTApi*> api
%type <ASTEnum*> enum
%type <ASTEnumConst*> enum_const

%start root

%%

root
    : api { $$ = $1; }
    | enum { throw syntax_error(@1, err_str<E2012>()); }
    | enum_const { throw syntax_error(@1, err_str<E2012>()); }
    | root api { throw syntax_error(@2, err_str<E2004>()); }
    | root enum { $1->enums.push_back($2); $2->parent = $1; $$ = $1; }
    | root enum_const { auto en = last_enum(@2); en->consts.push_back($2); $2->parent = en; $$ = $1; }
    ;

api
    : API ID { throw syntax_error(@1, err_str<E2005>()); }
    | doc API ID idoc { throw syntax_error(@2, err_str<E2021>()); }
    | doc API ID { 
        auto node = alloc_node(ASTApi, @2);
        node->name = $3;
        node->doc = $1;
        $1->parent = node;
        $$ = node;
    }
    | API ID idoc { 
        auto node = alloc_node(ASTApi, @1);
        node->name = $2;
        node->doc = $3;
        $3->parent = node;
        $$ = node;
    }
    ;

enum
    : ENUM ID { throw syntax_error(@1, err_str<E2005>()); }
    | doc ENUM ID idoc { throw syntax_error(@2, err_str<E2021>()); }
    | doc ENUM ID attr_list idoc { throw syntax_error(@2, err_str<E2021>()); }
    | doc ENUM ID {
        auto node = alloc_node(ASTEnum, @2);
        node->name = $3;
        node->doc = $1;
        $1->parent = node;
        $$ = node;
    }
    | ENUM ID idoc {
        auto node = alloc_node(ASTEnum, @1);
        node->name = $2;
        node->doc = $3;
        $3->parent = node;
        $$ = node;
    }
    | doc ENUM ID attr_list {
        auto node = alloc_node(ASTEnum, @2);
        node->name = $3;
        node->doc = $1;
        $1->parent = node;
        addAttrs(node, $4, { ASTAttr::Flags, ASTAttr::Hex, ASTAttr::Platform });
        $$ = node;
    }
    | ENUM ID attr_list idoc {
        auto node = alloc_node(ASTEnum, @1);
        node->name = $2;
        node->doc = $4;
        $4->parent = node;
        addAttrs(node, $3, { ASTAttr::Flags, ASTAttr::Hex, ASTAttr::Platform });
        $$ = node;
    }
    ;

enum_const
    : CONST ID { throw syntax_error(@1, err_str<E2005>()); }
    | doc CONST ID idoc { throw syntax_error(@2, err_str<E2021>()); }
    | doc CONST ID attr_list idoc { throw syntax_error(@2, err_str<E2021>()); }
    | doc CONST ID {
        auto node = alloc_node(ASTEnumConst, @2);
        node->name = $3;
        node->doc = $1;
        $1->parent = node;
        $$ = node;
    }
    | CONST ID idoc {
        auto node = alloc_node(ASTEnumConst, @1);
        node->name = $2;
        node->doc = $3;
        $3->parent = node;
        $$ = node;
    }
    | doc CONST ID attr_list {
        auto node = alloc_node(ASTEnumConst, @2);
        node->name = $3;
        node->doc = $1;
        $1->parent = node;
        addAttrs(node, $4, { ASTAttr::Value });
        $$ = node;
    }
    | CONST ID attr_list idoc {
        auto node = alloc_node(ASTEnumConst, @1);
        node->name = $2;
        node->doc = $4;
        $4->parent = node;
        addAttrs(node, $3, { ASTAttr::Value });
        $$ = node;
    }
    ;

attr_list
    : attr_item { auto list = std::vector<ASTAttr*>(); list.push_back($1); $$ = list; }
    | attr_list ',' attr_item { $1.push_back($3); $$ = $1; }
    ;

attr_item
    : ATTRFLAGS { auto node = alloc_node(ASTAttr, @1); node->type = ASTAttr::Flags; $$ = node; }
    | ATTRHEX { auto node = alloc_node(ASTAttr, @1); node->type = ASTAttr::Hex; $$ = node; }
    | ATTRPLATFORM { throw syntax_error(@1, err_str<E2016>()); }
    | ATTRPLATFORM attr_args {
        auto node = alloc_node(ASTAttr, @1);
        node->type = ASTAttr::Platform;
        addAttrPlatformArgs(node, $2);
        $$ = node;
    }
    | ATTRVALUE { throw syntax_error(@1, err_str<E2023>()); }
    | ATTRVALUE attr_args {
        auto node = alloc_node(ASTAttr, @1);
        node->type = ASTAttr::Value;
        addAttrValueArgs(node, $2);
        $$ = node;
    }
    ;

attr_args
    : '(' attr_arg_list ')' { $$ = $2; }
    ;

attr_arg_list
    : ATTRARG { auto list = std::vector<std::string>(); list.push_back($1); $$ = list; }
    | attr_arg_list ',' ATTRARG { $1.push_back($3); $$ = $1; }
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

void addAttrs(ASTDecl* node, const std::vector<ASTAttr*>& attrs, const std::set<ASTAttr::Type>& allowedAttrs)
{
    node->attrs = attrs;
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
    for (auto attr : node->attrs)
    {
        if (!allowedAttrs.contains(attr->type))
        {
            auto first = true;
            std::ostringstream ss;
            for (auto type : allowedAttrs) {
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

void addAttrValueArgs(ASTAttr* node, const std::vector<std::string>& values)
{
    if (values.size() != 1) 
    {
        throw idl::Parser::syntax_error(node->location, err_str<E2024>());
    }
    const auto& str = values[0];

    char* end;  
    long int result = std::strtol(str.c_str(), &end, 10);
    if (*end != '\0')
    {  
        throw idl::Parser::syntax_error(node->location, err_str<E2025>());
    }
    ASTAttr::Arg arg{};
    arg.value = result;
    node->args.push_back(arg);
}

void idl::Parser::error(const location_type& loc, const std::string& message)
{
   std::cerr << "error: " << message << " at " << Location(loc) << std::endl;
}
