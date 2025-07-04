%require "3.7.4"
%language "C++"
%defines "parser.hpp"
%output "parser.cpp"
%locations

%{
#include "idl.hpp"
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
        scanner.context().allocNode<ast>(loc)
    #define add_attrs(node, attrs) \
        scanner.context().addAttrs(node, attrs)
    #define intern(loc, str) \
        scanner.context().internStr(loc, std::string(str))
    #define intern_bool(loc, b) \
        scanner.context().internBool(loc, bool(b))
    #define intern_int(loc, num) \
        scanner.context().internInt(loc, int64_t(num))
    
    void addNode(idl::Context&, idl::ASTDecl*, idl::ASTDecl*);
    void addDoc(idl::ASTDoc*, const std::vector<idl::ASTNode*>&, char);
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
%token ATTRDATASIZE
%token ATTRCONST
%token ATTRREF
%token ATTRREFINC
%token ATTRUSERDATA
%token ATTRERRORCODE
%token ATTRNOERROR
%token ATTRRESULT
%token ATTRDESTROY
%token ATTRIN
%token ATTROUT
%token ATTROPTIONAL
%token ATTRTOKENIZER
%token ATTRVERSION

%token API
%token ENUM
%token CONST
%token STRUCT
%token FIELD
%token INTERFACE
%token METHOD
%token ARG
%token PROP
%token EVENT
%token HANDLE
%token FUNC
%token CALLBACK
%token FILEDOC

%token <std::string> STR
%token <std::string> ID
%token <std::string> REF
%token <std::string> TOKINDX
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
%type <ASTAttr*> attr_datasize
%type <ASTAttr*> attr_tokenizer
%type <ASTAttr*> attr_version
%type <ASTAttr*> attr_const
%type <ASTAttr*> attr_ref
%type <ASTAttr*> attr_refinc
%type <ASTAttr*> attr_userdata
%type <ASTAttr*> attr_errorcode
%type <ASTAttr*> attr_noerror
%type <ASTAttr*> attr_result
%type <ASTAttr*> attr_destroy
%type <ASTAttr*> attr_in
%type <ASTAttr*> attr_out
%type <ASTAttr*> attr_optional
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
    : def_with_attrs { err<IDL_STATUS_E2005>(@1, $1->fullname()); }
    | doc def_with_attrs idoc { err<IDL_STATUS_E2021>(@2); }
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
    | EVENT { auto node = alloc_node(ASTEvent, @1); $$ = node; }
    | HANDLE { auto node = alloc_node(ASTHandle, @1); $$ = node; }
    | FUNC { auto node = alloc_node(ASTFunc, @1); $$ = node; }
    | CALLBACK { auto node = alloc_node(ASTCallback, @1); $$ = node; }
    | FILEDOC { auto node = alloc_node(ASTFile, @1); $$ = node; }
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
    | attr_datasize { $$ = $1; }
    | attr_tokenizer { $$ = $1; }
    | attr_version { $$ = $1; }
    | attr_const { $$ = $1; }
    | attr_ref { $$ = $1; }
    | attr_refinc { $$ = $1; }
    | attr_userdata { $$ = $1; }
    | attr_errorcode { $$ = $1; }
    | attr_noerror { $$ = $1; }
    | attr_result { $$ = $1; }
    | attr_destroy { $$ = $1; }
    | attr_in { $$ = $1; }
    | attr_out { $$ = $1; }
    | attr_optional { $$ = $1; }
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
    : ATTRPLATFORM { err<IDL_STATUS_E2016>(@1); }
    | ATTRPLATFORM '(' ')' { err<IDL_STATUS_E2016>(@1); }
    | ATTRPLATFORM '(' attr_platform_arg_list ')' {
        auto node = alloc_node(ASTAttrPlatform, @1);
        node->platforms = $3;
        $$ = node;
    }
    ;

attr_value
    : ATTRVALUE { err<IDL_STATUS_E2023>(@1); }
    | ATTRVALUE '(' ')' { err<IDL_STATUS_E2023>(@1); }
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
    : ATTRTYPE { err<IDL_STATUS_E2049>(@1); }
    | ATTRTYPE '(' ')' { err<IDL_STATUS_E2049>(@1); }
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
    : ATTRCNAME { err<IDL_STATUS_E2075>(@1); }
    | ATTRCNAME '(' ')' { err<IDL_STATUS_E2075>(@1); }
    | ATTRCNAME '(' STR ')' {
        auto node = alloc_node(ASTAttrCName, @1);
        node->name = $3;
        $$ = node;
    }
    ;

attr_array
    : ATTRARRAY { err<IDL_STATUS_E2076>(@1); }
    | ATTRARRAY '(' ')' { err<IDL_STATUS_E2076>(@1); }
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

attr_datasize
    : ATTRDATASIZE { err<IDL_STATUS_E2112>(@1); }
    | ATTRDATASIZE '(' ')' { err<IDL_STATUS_E2112>(@1); }
    | ATTRDATASIZE '(' REF ')' {
        auto ref = alloc_node(ASTDeclRef, @3);
        ref->name = $3;
        auto node = alloc_node(ASTAttrDataSize, @1);
        node->decl = ref;
        ref->parent = node;
        $$ = node;
    }
    ;

attr_tokenizer
    : ATTRTOKENIZER { err<IDL_STATUS_E2109>(@1); }
    | ATTRTOKENIZER '(' ')' { err<IDL_STATUS_E2109>(@1); }
    | ATTRTOKENIZER '(' TOKINDX ')' {
        std::vector<int> tokens;  
        std::stringstream ss($3);  
        std::string token;  
        while (std::getline(ss, token, '-')) {  
            if (token[0] == '^') {
                tokens.push_back(-std::stoi(token.substr(1)));  
            } else {
                tokens.push_back(std::stoi(token));  
            }
        }
        auto node = alloc_node(ASTAttrTokenizer, @1);
        node->nums = tokens;
        $$ = node;
    }
    ;

attr_version
    : ATTRVERSION { err<IDL_STATUS_E2110>(@1); }
    | ATTRVERSION '(' ')' { err<IDL_STATUS_E2110>(@1); }
    | ATTRVERSION '(' NUM ',' NUM ',' NUM ')' {
        auto node = alloc_node(ASTAttrVersion, @1);
        node->major = $3;
        node->minor = $5;
        node->micro = $7;
        $$ = node;
    }
    ;

attr_const
    : ATTRCONST { auto node = alloc_node(ASTAttrConst, @1); $$ = node; }
    ;

attr_ref
    : ATTRREF { auto node = alloc_node(ASTAttrRef, @1); $$ = node; }
    ;

attr_refinc
    : ATTRREFINC { auto node = alloc_node(ASTAttrRefInc, @1); $$ = node; }
    ;

attr_userdata
    : ATTRUSERDATA { auto node = alloc_node(ASTAttrUserData, @1); $$ = node; }
    ;

attr_errorcode
    : ATTRERRORCODE { auto node = alloc_node(ASTAttrErrorCode, @1); $$ = node; }
    ;

attr_noerror
    : ATTRNOERROR { auto node = alloc_node(ASTAttrNoError, @1); $$ = node; }
    ;

attr_result
    : ATTRRESULT { auto node = alloc_node(ASTAttrResult, @1); $$ = node; }
    ;

attr_destroy
    : ATTRDESTROY { auto node = alloc_node(ASTAttrDestroy, @1); $$ = node; }
    ;

attr_in
    : ATTRIN { auto node = alloc_node(ASTAttrIn, @1); $$ = node; }
    ;

attr_out
    : ATTROUT { auto node = alloc_node(ASTAttrOut, @1); $$ = node; }
    ;

attr_optional
    : ATTROPTIONAL { auto node = alloc_node(ASTAttrOptional, @1); $$ = node; }
    ;

attr_get
    : ATTRGET { err<IDL_STATUS_E2050>(@1); }
    | ATTRGET '(' ')' { err<IDL_STATUS_E2050>(@1); }
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
    : ATTRSET { err<IDL_STATUS_E2050>(@1); }
    | ATTRSET '(' ')' { err<IDL_STATUS_E2050>(@1); }
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
            err<IDL_STATUS_E2018>(@3, str);
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
    : DOC { err<IDL_STATUS_E2006>(@$); }
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
    : IDOC { err<IDL_STATUS_E2006>(@$); }
    | IDOC doc_field           { $$ = std::make_pair($2, 'd'); }
    | IDOC doc_field DOCDETAIL { $$ = std::make_pair($2, 'd'); }
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

void addNode(idl::Context& context, idl::ASTDecl* prev, idl::ASTDecl* decl)
{
    if (prev == nullptr)
    {
        if (!decl->is<idl::ASTApi>())
        {
            err<IDL_STATUS_E2012>(decl->location);
        }
        context.initBuiltins();
        return;
    }
    if (decl->is<idl::ASTApi>())
    {
        err<IDL_STATUS_E2004>(decl->location);
    }
    
    idl::ChildsAggregator aggregator = prev;
    decl->accept(aggregator);
    if (auto file = decl->as<idl::ASTFile>())
    {
        context.pushFile(file);
    }
    else 
    {
        context.addSymbol(decl);
    }
}

std::vector<idl::ASTNode*> prepareDoc(const std::vector<idl::ASTNode*>& doc) {
    std::vector<idl::ASTNode*> result;
    result.reserve(doc.size());
    for (size_t i = 0; i < doc.size(); ++i)
    {
        if (i == 0 && doc[i]->is<idl::ASTLiteralStr>() && doc[i]->as<idl::ASTLiteralStr>()->value[0] == ' ')
        {
        }
        else if (i + 1 == doc.size() && doc[i]->is<idl::ASTLiteralStr>() && doc[i]->as<idl::ASTLiteralStr>()->value[0] == ' ')
        {
        }
        else if (i + 1 < doc.size() && 
            doc[i]->is<idl::ASTLiteralStr>() && doc[i]->as<idl::ASTLiteralStr>()->value[0] == ' ' && 
            doc[i + 1]->is<idl::ASTLiteralStr>() && doc[i + 1]->as<idl::ASTLiteralStr>()->value[0] == '\n')
        {
        }
        else
        {
            result.push_back(doc[i]);
        }
    }
    return result;
}

void addDoc(idl::ASTDoc* node, const std::vector<idl::ASTNode*>& doc, char type)
{
    auto field = prepareDoc(doc);
    switch (type)
    {
        case 'b':
            if (!node->brief.empty()) err<IDL_STATUS_E2007>(node->location);
            node->brief = field;
            break;
        case 'd':
            if (!node->detail.empty()) err<IDL_STATUS_E2008>(node->location);
            node->detail = field;
            break;
        case 'c':
            if (!node->copyright.empty()) err<IDL_STATUS_E2009>(node->location);
            node->copyright = field;
            break;
        case 'l':
            if (!node->license.empty()) err<IDL_STATUS_E2010>(node->location);
            node->license = field;
            break;
        case 'a':
            node->authors.push_back(field);
            break;
        case 's':
            node->see.push_back(field);
            break;
        case 'n':
            node->note.push_back(field);
            break;
        case 'w':
            node->warn.push_back(field);
            break;
        case 'r':
            node->ret = field;
            break;
    }
}

void idl::Parser::error(const location_type& loc, const std::string& message)
{
    err<IDL_STATUS_E2011>(loc);
}
