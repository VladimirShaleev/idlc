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
        scanner.context().allocNode<ast, idl::Parser::syntax_error>(loc)
    #define alloc_attribute(loc, name, args) \
        scanner.context().allocAttribute<idl::Parser::syntax_error>(loc, name, args)
}

%initial-action {
    @$.begin.filename = @$.end.filename = scanner.filename();
}

%token NAMESPACE
%token ENUM
%token INTERFACE
%token <std::string> ID
%token <int64_t> NUM

%type <ASTNode*> program
%type <ASTNamespace*> namespace_decl
%type <ASTDecl*> enum_decl
%type <ASTDecl*> interface_decl
%type <std::vector<ASTDecl*>> declarations

// enums
%type <std::pair<std::string, std::optional<int64_t>>> enum_const
%type <std::vector<ASTEnumConst*>> enum_const_list

// methods
%type <ASTMethod*> method_decl
%type <std::vector<ASTMethod*>> method_list
%type <std::vector<ASTParameter*>> param_list
%type <ASTParameter*> param_decl

// attributes
%type <std::vector<ASTAttribute*>> attribute_list
%type <std::vector<ASTAttribute*>> attribute_item_list
%type <ASTAttribute*> attribute;
%type <std::vector<std::string>> attribute_args

// refs
%type <ASTDeclRef*> ref

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

// enums
enum_decl : 
    attribute_list ENUM ID '{' enum_const_list '}' {
        auto en = alloc_node(ASTEnum, @3);
        en->name = $3;
        en->attributes = $1;
        for (auto attr : $1) {
            if (attr->type != ASTAttribute::Flags && 
                attr->type != ASTAttribute::Platform &&
                attr->type != ASTAttribute::Hex) {
                throw syntax_error(@1, "enums only support the following attributes: [flags,hex,platform]");
            }
            attr->parent = en;
        }
        for (auto ec : $5) {
            ec->parent = en;
            en->constants.push_back(ec);
        }
        $$ = en;
    }
    | ENUM ID '{' enum_const_list '}' {
        auto en = alloc_node(ASTEnum, @2);
        en->name = $2;
        for (auto ec : $4) {
            ec->parent = en;
            en->constants.push_back(ec);
        }
        $$ = en;
    }
    ;

enum_const_list : 
    enum_const { 
        std::vector<ASTEnumConst*> list;
        auto ec = alloc_node(ASTEnumConst, @1);
        ec->name = $1.first;
        ec->value = $1.second.value_or(0);
        if (ec->value < INT32_MIN || ec->value > INT32_MAX) {
            throw syntax_error(@1, "the value of the constant must be in the range [-2147483648, 2147483647]");
        }
        list.push_back(ec);
        $$ = list;
    }
    | enum_const_list ',' enum_const {
        auto ec = alloc_node(ASTEnumConst, @3);
        ec->name = $3.first;
        ec->value = $3.second.value_or($1.back()->value + 1);
        if (ec->value < INT32_MIN || ec->value > INT32_MAX) {
            throw syntax_error(@3, "the value of the constant must be in the range [-2147483648, 2147483647]");
        }
        $1.push_back(ec);
        $$ = $1;
    }
    ;

enum_const
    : ID { $$ = std::make_pair($1, std::nullopt); }
    | ID '=' NUM { $$ = std::make_pair($1, $3); }
    ;

// interfaces
interface_decl : 
    attribute_list INTERFACE ID '{' method_list '}' {
        auto iface = alloc_node(ASTInterface, @3);
        iface->name = $3;
        iface->attributes = $1;
        iface->methods = $5;
        for (auto method : $5) {
            method->parent = iface;
        }
        for (auto attr : $1) {
            if (attr->type != ASTAttribute::Platform) {
                throw syntax_error(@1, "interfaces only support the following attributes: [platform]");
            }
            attr->parent = iface;
        }
        $$ = iface;
    }
    | INTERFACE ID '{' method_list '}' {
        auto iface = alloc_node(ASTInterface, @2);
        iface->name = $2;
        iface->methods = $4;
        for (auto method : $4) {
            method->parent = iface;
        }
        $$ = iface;
    }
    ;

// methods
method_list
    : method_decl { std::vector<ASTMethod*> list; list.push_back($1); $$ = list; }
    | method_list ',' method_decl { $1.push_back($3); $$ = $1; }
    ;

method_decl : 
    ref ID '(' param_list ')' ';' {
        auto method = alloc_node(ASTMethod, @2);
        method->returnTypeRef = $1;
        method->name = $2;
        method->parameters = $4;
        method->returnTypeRef->parent = method;
        for (auto param : $4) {
            param->parent = method;
        }
        $$ = method;
    }
    ;

param_list : 
    /* empty */ { $$ = std::vector<ASTParameter*>(); }
    | param_decl { std::vector<ASTParameter*> list; list.push_back($1); $$ = list; }
    | param_list ',' param_decl { $1.push_back($3); $$ = $1; }
    ;

param_decl : 
    attribute_list ref ID {
        auto param = alloc_node(ASTParameter, @2);
        param->typeRef = $2;
        param->name    = $3;
        param->attributes = $1;
        param->typeRef->parent = param;
        for (auto attr : $1) {
            if (attr->type != ASTAttribute::In && attr->type != ASTAttribute::Out) {
                throw syntax_error(@1, "params only support the following attributes: [in,out]");
            }
            attr->parent = param;
        }
        $$ = param;
    }
    | ref ID {
        auto param = alloc_node(ASTParameter, @1);
        param->typeRef = $1;
        param->name    = $2;
        param->typeRef->parent = param;
        param->attributes.push_back(alloc_attribute(@1, "in", {}));
        param->attributes.back()->parent = param;
        $$ = param;
    }
    ;

// attributes
attribute_list
    : '[' attribute_item_list ']' { $$ = $2; }
    | '[' ']' { throw syntax_error(@2, "empty attribute list is not allowed in attributes"); }
    ;

attribute_item_list :
    attribute  {
        std::vector<ASTAttribute*> list;
        list.push_back($1);
        $$ = list;
    }
    | attribute_item_list ',' attribute {
        auto attr = $3;
        auto it = std::find_if($1.begin(), $1.end(), [attr](const auto& item) {
            return item->type == attr->type;
        });
        if (it != $1.end()) {
            throw syntax_error(@3, "duplicate attribute");
        }
        $1.push_back(attr);
        $$ = $1;
    }
    ;

attribute
    : ID { $$ = alloc_attribute(@1, $1, {}); }
    | ID '(' attribute_args ')' { $$ = alloc_attribute(@1, $1, $3); }
    | ID '(' ')' { throw syntax_error(@3, "empty argument list is not allowed in attributes"); }
    ;

attribute_args
    : ID { std::vector<std::string> list; list.push_back($1); $$ = list; }
    | attribute_args ',' ID { $1.push_back($3); $$ = $1; }
    ;

// refs
ref :
    ID {
        auto ref = alloc_node(ASTDeclRef, @1);
        ref->name = $1;
        $$ = ref;
    }
    ;

%%

void idl::Parser::error(const location_type& loc, const std::string& message)
{
   std::cerr << "error: " << message << " at " << Location(loc) << std::endl;
}
