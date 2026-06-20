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
%x REFCTX
%x IMPORTCTX

DOCCHAR ([^ \r\n\t\{\}[\]]|\\\{|\\\}|\\\[|\\\])

%%

%{
    yylloc->step();
%}

"api"   { setDeclaring(); return token::API; }
"enum"  { setDeclaring(); return token::ENUM; }
"const" { setDeclaring(); return token::CONST; }

"["                    { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRCTX>"("           { BEGIN(ATTRARGCTX); return YYText()[0]; }
<ATTRCTX>"version"     { return token::ATTRVERSION; }
<ATTRCTX>"flags"       { return token::ATTRFLAGS; }
<ATTRCTX>"hex"         { return token::ATTRHEX; }
<ATTRCTX>"brief"       { return token::ATTRBRIEF; }
<ATTRCTX>"detail"      { return token::ATTRDETAIL; }
<ATTRCTX>"value"       { return token::ATTRVALUE; }
<ATTRCTX>"type"        { return token::ATTRTYPE; }
<ATTRCTX>"cname"       { return token::ATTRCNAME; }
<ATTRCTX>"tokenizer"   { return token::ATTRTOKENIZER; }
<ATTRCTX>","           { return YYText()[0]; }
<ATTRCTX>[a-zA-Z0-9_]+ { yylval->emplace<std::string>(YYText()); return token::INVALID_ATTR; }
<ATTRCTX>"]"           { BEGIN(INITIAL); return YYText()[0]; }

<ATTRARGCTX>","                 { return YYText()[0]; }
<ATTRARGCTX>" "                 { }
<ATTRARGCTX>[a-z_][a-zA-Z0-9_]* { yylval->emplace<std::string>(YYText()); return token::INVALID_ARG; }
<ATTRARGCTX>")"                 { BEGIN(ATTRCTX); return YYText()[0]; }

"@"                { BEGIN(DOCCTX); return isDeclaring() ? token::IDOC : token::DOC; }
<DOCCTX>{DOCCHAR}+ { yylval->emplace<std::string>(unescape(trim(YYText()))); return token::STR; }
<DOCCTX>"{"        { BEGIN(REFCTX); return YYText()[0]; }
<DOCCTX>\[         { BEGIN(INITIAL); yyless(yyleng - 1); }
<DOCCTX>\r?\n      { yylloc->lines(); setDeclaring(false); BEGIN(INITIAL); }
<REFCTX>"}"        { BEGIN(DOCCTX); return YYText()[0]; }

"b166074c3cba4005a198513772597880" { setDeclaring(); return token::IMPORT; }
import[ ]+ { BEGIN(IMPORTCTX); }
<IMPORTCTX>[-\.a-zA-Z0-9_]+ {
    std::string importName = YYText();
    int c;
    while ((c = yyinput()) && c != '\n') {
        if (c != ' ' && c != '\r') {
            context().log<IDL_STATUS_E3020>(*yylloc);
        }
    }
    yylloc->lines();
    import(*yylloc, yytext);
    BEGIN(INITIAL);
    unput('\n'), unput('\n');
    for (auto it = importName.rbegin(); it != importName.rend(); ++it) {
        unput(*it);
    }
    unput(' ');
    unput('0'), unput('8'), unput('8'), unput('7'), unput('9'), unput('5'), unput('2'), unput('7');
    unput('7'), unput('3'), unput('1'), unput('5'), unput('8'), unput('9'), unput('1'), unput('a');
    unput('5'), unput('0'), unput('0'), unput('4'), unput('a'), unput('b'), unput('c'), unput('3');
    unput('c'), unput('4'), unput('7'), unput('0'), unput('6'), unput('6'), unput('1'), unput('b');
}
<IMPORTCTX>.|\r?\n { context().log<IDL_STATUS_E3019>(*yylloc, YYText()); }

"//".* ;

":" { return YYText()[0]; }
"," { return YYText()[0]; }

<*>[A-Z][a-zA-Z0-9]*                     { yylval->emplace<std::string>(YYText()); return token::ID; }
<*>\"(\\.|[^\\"\n])*\"                   { std::string str = YYText(); str = str.substr(1, str.length() - 2); yylval->emplace<std::string>(str); return token::STR; }
<*>\"(\\.|[^\\"\n])*                     { context().log<IDL_STATUS_E3009>(*yylloc, YYText()); }
<*>[-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)? { yylval->emplace<double>(std::stof(YYText())); return token::FLOAT; }
<*>[-+]?[0-9]+                           { yylval->emplace<int64_t>(std::stoll(YYText())); return token::INT; }

<*><<EOF>>                { setDeclaring(false); if (popImport()) { return token::POPIMPORT; } else { return token::YYEOF; } }
<*>\r?\n                  { yylloc->lines(); setDeclaring(false); }
<*>\t                     { context().log<IDL_STATUS_E3020>(*yylloc); }
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
