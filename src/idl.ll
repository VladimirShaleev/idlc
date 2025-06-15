%{
#include <sstream>
#include "errors.hpp"
#include "scanner.hpp"
#define YY_NO_UNISTD_H
#define YY_DECL int idl::Scanner::yylex(idl::Parser::semantic_type* yylval, idl::Parser::location_type* yylloc)
#define YY_USER_ACTION yylloc->step(); yylloc->columns(yyleng);
using namespace std::string_literals;
typedef idl::Parser::token token;
std::string unescape(const char*);
%}

%option c++
%option noyywrap
%option yylineno
%option nodefault
%option outfile="scanner.cpp"

%x DOCSTR
%x DOCMSTR
%x ATTRCTX
%x ATTRARGS

DOCCHAR  ([^ \r\n\t\{\}[\]]|\\\{|\\\}|\\\[|\\\])
DOCMCHAR ([^ \r\n\t\{\}[\]`]|^[`]{3}|\\\{|\\\}|\\\[|\\\])

%%

%{
    yylloc->step();
%}

"api"   { context().currentDeclLine(yylloc->end.line); return token::API; }
"enum"  { context().currentDeclLine(yylloc->end.line); return token::ENUM; }
"const" { context().currentDeclLine(yylloc->end.line); return token::CONST; }

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
<ATTRCTX>"platform" { return token::ATTRPLATFORM; }
<ATTRCTX>"value"    { return token::ATTRVALUE; }
<ATTRCTX>","        { return YYText()[0]; }
<ATTRCTX>" "        ;
<ATTRCTX>[a-z]+     { err<E2015>(*yylloc, YYText()); }
<ATTRCTX>[^\]\(]    { err<E2001>(*yylloc, YYText()); }
<ATTRCTX>"]"        { BEGIN(INITIAL); return YYText()[0]; }
<ATTRCTX>"("        { BEGIN(ATTRARGS); return YYText()[0]; }

<ATTRARGS>[-+]?[a-z0-9]+ { yylval->emplace<std::string>(YYText()); return token::ATTRARG; }
<ATTRARGS>","            { return YYText()[0]; }
<ATTRARGS>")"            { BEGIN(ATTRCTX); return YYText()[0]; }

[A-Z][a-zA-Z0-9]* { yylval->emplace<std::string>(YYText()); return token::ID; }
[-+]?[0-9]+       { yylval->emplace<int64_t>(std::stoll(YYText())); return token::NUM; }
[a-zA-Z0-9]+      { err<E2003>(*yylloc, YYText()); }
<<EOF>>           { return token::YYEOF; }
":"               { return YYText()[0]; }
\n                { yylloc->lines(); }
\t                { err<E2002>(*yylloc); }
" "               ;
"//".*            ;
.                 { err<E2001>(*yylloc, YYText()); }

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
