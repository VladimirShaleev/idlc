%{
#include <sstream>
#include "errors.hpp"
#include "scanner.hpp"
#define YY_NO_UNISTD_H
#define YY_DECL int idl::Scanner::yylex(idl::Parser::semantic_type* yylval, idl::Parser::location_type* yylloc)
#define YY_USER_ACTION action(*yylloc);
using namespace std::string_literals;
typedef idl::Parser::token token;
std::string unescape(const char*);
%}

%option c++
%option noyywrap
%option yylineno
%option outfile="scanner.cpp"

%x DOCSTR
%x DOCMSTR
%x ATTRCTX
%x ATTRARGTYPE
%x ATTRARGVALUE
%x ATTRARGPATFORM
%x TYPE
%x DECLREF
%x IMPORT

DOCCHAR  ([^ \r\n\t\{\}[\]]|\\\{|\\\}|\\\[|\\\])
DOCMCHAR ([^ \r\n\t\{\}[\]`]|^[`]{3}|\\\{|\\\}|\\\[|\\\])

%%

%{
    yylloc->step();
%}

"api"       { context().currentDeclLine(yylloc->end.line); return token::API; }
"enum"      { context().currentDeclLine(yylloc->end.line); return token::ENUM; }
"const"     { context().currentDeclLine(yylloc->end.line); return token::CONST; }
"struct"    { context().currentDeclLine(yylloc->end.line); return token::STRUCT; }
"field"     { context().currentDeclLine(yylloc->end.line); return token::FIELD; }
"interface" { context().currentDeclLine(yylloc->end.line); return token::INTERFACE; }
"method"    { context().currentDeclLine(yylloc->end.line); return token::METHOD; }
"arg"       { context().currentDeclLine(yylloc->end.line); return token::ARG; }

"@" {
    BEGIN(DOCSTR);
    auto prevLine = context().currentDeclLine();
    auto currLine = yylloc->end.line;
    return prevLine == currLine ?  token::IDOC : token::DOC;
}
<DOCSTR>{DOCCHAR}+         { yylval->emplace<std::string>(unescape(YYText())); return token::STR; }
<DOCSTR>"[brief]"[ ]*$     { return token::DOCBRIEF; }
<DOCSTR>"[detail]"[ ]*$    { return token::DOCDETAIL; }
<DOCSTR>"[author]"[ ]*$    { return token::DOCAUTHOR; }
<DOCSTR>"[copyright]"[ ]*$ { return token::DOCCOPYRIGHT; }
<DOCSTR>"[license]"[ ]*$   { return token::DOCLICENSE; }
<DOCSTR>"[return]"[ ]*$    { return token::DOCRETURN; }
<DOCSTR>"[".*"]"[ ]*$      { err<E2020>(*yylloc, YYText()); }
<DOCSTR>[\{\}]             { return YYText()[0]; }
<DOCSTR>\n                 { yylloc->lines(); BEGIN(INITIAL); }
<DOCSTR>\t                 { err<E2002>(*yylloc); }
<DOCSTR>" "                ;
<DOCSTR>.                  { err<E2001>(*yylloc, YYText()); }
<DOCSTR>"```\n"            { yylloc->lines(); BEGIN(DOCMSTR); }

<DOCMSTR>{DOCMCHAR}+ { yylval->emplace<std::string>(unescape(YYText())); return token::STR; }
<DOCMSTR>[\{\}] { return YYText()[0]; }
<DOCMSTR>^[ ]+  ;
<DOCMSTR>\n     { yylloc->lines(); yylval->emplace<std::string>(YYText()); return token::STR; }
<DOCMSTR>\t     { err<E2002>(*yylloc); }
<DOCMSTR>" "    ;
<DOCMSTR>.      { err<E2001>(*yylloc, YYText()); }
<DOCMSTR>"```"  { BEGIN(DOCSTR); }

"["                 { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRCTX>"flags"    { return token::ATTRFLAGS; }
<ATTRCTX>"hex"      { return token::ATTRHEX; }
<ATTRCTX>"platform" { BEGIN(ATTRARGPATFORM); return token::ATTRPLATFORM; }
<ATTRCTX>"value"    { BEGIN(ATTRARGVALUE); return token::ATTRVALUE; }
<ATTRCTX>"type"     { BEGIN(ATTRARGTYPE); return token::ATTRTYPE; }
<ATTRCTX>"static"   { return token::ATTRSTATIC; }
<ATTRCTX>"ctor"     { return token::ATTRCTOR; }
<ATTRCTX>","        { return YYText()[0]; }
<ATTRCTX>" "        ;
<ATTRCTX>\n         { yylloc->lines(); }
<ATTRCTX>[a-z]+     { err<E2015>(*yylloc, YYText()); }
<ATTRCTX>[^\]\(]    { err<E2001>(*yylloc, YYText()); }
<ATTRCTX>"]"        { BEGIN(INITIAL); return YYText()[0]; }

<ATTRARGVALUE>"("                 { return YYText()[0]; }
<ATTRARGVALUE>")"                 { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGVALUE>[-+]?[0-9]+         { yylval->emplace<int64_t>(std::stoll(YYText())); return token::NUM; }
<ATTRARGVALUE>"true"              { yylval->emplace<bool>(true); return token::BOOL; }
<ATTRARGVALUE>"false"             { yylval->emplace<bool>(false); return token::BOOL; }
<ATTRARGVALUE>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<ATTRARGVALUE>","                 { return YYText()[0]; }
<ATTRARGVALUE>" "                 ;
<ATTRARGVALUE>\n                  { yylloc->lines(); }
<ATTRARGVALUE>.                   { err<E2001>(*yylloc, YYText()); }

<ATTRARGPATFORM>"("       { return YYText()[0]; }
<ATTRARGPATFORM>")"       { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGPATFORM>"windows" { yylval->emplace<ASTAttrPlatform::Type>(ASTAttrPlatform::Windows); return token::ATTRPLATFORMARG; }
<ATTRARGPATFORM>"linux"   { yylval->emplace<ASTAttrPlatform::Type>(ASTAttrPlatform::Linux); return token::ATTRPLATFORMARG; }
<ATTRARGPATFORM>"macos"   { yylval->emplace<ASTAttrPlatform::Type>(ASTAttrPlatform::MacOS); return token::ATTRPLATFORMARG; }
<ATTRARGPATFORM>"web"     { yylval->emplace<ASTAttrPlatform::Type>(ASTAttrPlatform::Web); return token::ATTRPLATFORMARG; }
<ATTRARGPATFORM>"android" { yylval->emplace<ASTAttrPlatform::Type>(ASTAttrPlatform::Android); return token::ATTRPLATFORMARG; }
<ATTRARGPATFORM>"ios"     { yylval->emplace<ASTAttrPlatform::Type>(ASTAttrPlatform::iOS); return token::ATTRPLATFORMARG; }
<ATTRARGPATFORM>","       { return YYText()[0]; }
<ATTRARGPATFORM>" "       ;
<ATTRARGPATFORM>\n        { yylloc->lines(); }
<ATTRARGPATFORM>.         { err<E2017>(*yylloc, "'windows', 'linux', 'macos', 'web', 'android', 'ios"); }

<ATTRARGTYPE>"("                 { return YYText()[0]; }
<ATTRARGTYPE>")"                 { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGTYPE>" "                 ;
<ATTRARGTYPE>\n                  { yylloc->lines(); }
<ATTRARGTYPE>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<ATTRARGTYPE>.                   { err<E2001>(*yylloc, YYText()); }

import[ ]+ { BEGIN(IMPORT); }
<IMPORT>[-\.a-zA-Z0-9_]+ { 
    int c;
    while ((c = yyinput()) && c != '\n') {
        if (c != ' ') {
            err<E2001>(*yylloc, YYText());
        }
    }
    yylloc->lines();
    import(*yylloc, yytext);
    BEGIN(INITIAL);
}
<IMPORT>.|\n { err<E2001>(*yylloc, YYText()); }

<TYPE>" "                 ;
<TYPE>\n                  { yylloc->lines(); }
<TYPE>"}"                 { BEGIN(INITIAL); return YYText()[0]; }
<TYPE>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<TYPE>.                   { err<E2001>(*yylloc, YYText()); }

<DECLREF>" " ;
<DECLREF>\n                  { yylloc->lines(); }
<DECLREF>[A-Z][a-zA-Z0-9\.]* { BEGIN(INITIAL); yylval->emplace<std::string>(YYText()); return token::REF; }
<DECLREF>.                   { err<E2001>(*yylloc, YYText()); }

[A-Z][a-zA-Z0-9]*         { yylval->emplace<std::string>(YYText()); return token::ID; }
[-+]?[0-9]+               { yylval->emplace<int64_t>(std::stoll(YYText())); return token::NUM; }
"true"                    { yylval->emplace<bool>(true); return token::BOOL; }
"false"                   { yylval->emplace<bool>(false); return token::BOOL; }
[a-zA-Z0-9]+              { err<E2003>(*yylloc, YYText()); }
<<EOF>>                   { if (!popImport()) { return token::YYEOF; } }
\n                        { yylloc->lines(); }
\t                        { err<E2002>(*yylloc); }
" "                       ;
":"                       { BEGIN(DECLREF); return YYText()[0]; }
"{"                       { BEGIN(TYPE); return YYText()[0]; }
"//".*                    ;
.                         { err<E2001>(*yylloc, YYText()); }

%%

int yyFlexLexer::yylex() {
    throw std::runtime_error("Bad call to yyFlexLexer::yylex()");
}

std::string unescape(const char* str) {
    std::ostringstream ss;
    char c;
    while ((c = *str++) != '\0') {
        char nc = *str;
        if (c == '\\' && nc != '\0') {
            if (nc == '{' || nc == '}' || nc == '[' || nc == ']') {
                continue;
            }
        }
        ss << c;
    }
    return ss.str();
}
