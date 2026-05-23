%{
#include <sstream>
#include "scanner.hpp"
#define YY_NO_UNISTD_H
#define YY_DECL int idl::Scanner::yylex(idl::Parser::semantic_type* yylval, idl::Parser::location_type* yylloc)
#define YY_USER_ACTION action(*yylloc);
using namespace std::string_literals;
typedef idl::Parser::token token;
std::string trim(const std::string& str);
std::string unescape(const std::string& str);
%}

%option c++
%option noyywrap
%option yylineno
%option outfile="scanner.cpp"

%x ATTRCTX
%x ATTRARGCTX
%x DOCCTX

DOCCHAR ([^ \r\n\t\{\}[\]]|\\\{|\\\}|\\\[|\\\])

%%

%{
    yylloc->step();
%}

"api"  { return token::API; }
"enum" { return token::ENUM; }

"["                    { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRCTX>"("           { BEGIN(ATTRARGCTX); return YYText()[0]; }
<ATTRCTX>"version"     { return token::ATTRVERSION; }
<ATTRCTX>"flags"       { return token::ATTRFLAGS; }
<ATTRCTX>"hex"         { return token::ATTRHEX; }
<ATTRCTX>"brief"       { return token::ATTRBRIEF; }
<ATTRCTX>","           { return YYText()[0]; }
<ATTRCTX>[a-zA-Z0-9_]+ { yylval->emplace<std::string>(YYText()); return token::INVALID_ATTR; }
<ATTRCTX>"]"           { BEGIN(INITIAL); return YYText()[0]; }

<ATTRARGCTX>","                    { return YYText()[0]; }
<ATTRARGCTX>" "                    { }
<ATTRARGCTX>[a-zA-Z_][a-zA-Z0-9_]* { yylval->emplace<std::string>(YYText()); return token::INVALID_ARG; }
<ATTRARGCTX>")"                    { BEGIN(ATTRCTX); return YYText()[0]; }

"@"                { BEGIN(DOCCTX); return YYText()[0]; }
<DOCCTX>{DOCCHAR}+ { yylval->emplace<std::string>(unescape(trim(YYText()))); return token::STR; }
<DOCCTX>[\{\}]     { return YYText()[0]; }
<DOCCTX>\[         { BEGIN(INITIAL); yyless(yyleng - 1); }
<DOCCTX>\r?\n      { yylloc->lines(); BEGIN(INITIAL); }

"//".* ;

<*>[A-Z][a-zA-Z0-9]*                     { yylval->emplace<std::string>(YYText()); return token::ID; }
<*>\"(\\.|[^\\"\n])*\"                   { std::string str = YYText(); str = str.substr(1, str.length() - 2); yylval->emplace<std::string>(str); return token::STR; }
<*>\"(\\.|[^\\"\n])*                     { context().log<IDL_STATUS_E3009>(*yylloc, YYText()); }
<*>[-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)? { yylval->emplace<double>(std::stof(YYText())); return token::FLOAT; }
<*>[-+]?[0-9]+                           { yylval->emplace<int64_t>(std::stoll(YYText())); return token::INT; }

<*><<EOF>>                { return token::YYEOF; }
<*>\r?\n                  { yylloc->lines(); }
<*>\t                     { /* err<IDL_STATUS_E2002>(*yylloc); */ }
<*>" "                    { }
<*>[A-Za-z0-9]+           { context().log<IDL_STATUS_E3001>(*yylloc, YYText()); }
<*>.                      { context().log<IDL_STATUS_E3001>(*yylloc, YYText()); }

%%

int yyFlexLexer::yylex() {
    throw std::runtime_error("Bad call to yyFlexLexer::yylex()");
}

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string unescape(const std::string& str) {
    auto cStr = str.c_str();
    std::ostringstream ss;
    char c;
    while ((c = *cStr++) != '\0') {
        char nc = *cStr;
        if (c == '\\' && nc != '\0') {
            if (nc == '{' || nc == '}' || nc == '[' || nc == ']') {
                continue;
            }
        }
        ss << c;
    }
    return ss.str();
}
