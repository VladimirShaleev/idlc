%{
#include <sstream>
#include "scanner.hpp"
#define YY_NO_UNISTD_H
#define YY_DECL int idl::Scanner::yylex(idl::Parser::semantic_type* yylval, idl::Parser::location_type* yylloc)
#define YY_USER_ACTION action(*yylloc);
#define log(status, ...) context().log<IDL_STATUS_##status>({ \
    context().intern({yylloc->begin.filename->c_str(), yylloc->begin.filename->length()}), \
    uint16_t(yylloc->begin.column), \
    uint16_t(yylloc->begin.line) } __VA_OPT__(,) __VA_ARGS__)
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
%x ATTRSHORTARGCTX
%x ATTRFALLBACKARGCTX
%x DOCCTX
%x REFCTX
%x IMPORTCTX

DOCCHAR ([^ \r\n\t\{\}[\]]|\\\{|\\\}|\\\[|\\\])
FLOAT   [-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
INT     [-+]?[0-9]+
SYMBOL  [a-zA-Z0-9_\-^\.@]
SYMBOLS [a-zA-Z0-9_\-^\.@ ]

%%

%{
    yylloc->step();
%}

"api"   { setDeclaring(); return token::API; }
"enum"  { setDeclaring(); return token::ENUM; }
"const" { setDeclaring(); return token::CONST; }

"["                    { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRCTX>"("           { getAttrArg() == Default ? BEGIN(ATTRARGCTX) : getAttrArg() == ShortString ? BEGIN(ATTRSHORTARGCTX) : BEGIN(ATTRFALLBACKARGCTX); return YYText()[0]; }
<ATTRCTX>"version"     { attrArg(FallbackString); return token::ATTRVERSION; }
<ATTRCTX>"author"      { attrArg(); return token::ATTRAUTHOR; }
<ATTRCTX>"copyright"   { attrArg(); return token::ATTRCOPYRIGHT; }
<ATTRCTX>"license"     { attrArg(); return token::ATTRLICENSE; }
<ATTRCTX>"flags"       { attrArg(); return token::ATTRFLAGS; }
<ATTRCTX>"hex"         { attrArg(); return token::ATTRHEX; }
<ATTRCTX>"brief"       { attrArg(); return token::ATTRBRIEF; }
<ATTRCTX>"detail"      { attrArg(); return token::ATTRDETAIL; }
<ATTRCTX>"value"       { attrArg(); return token::ATTRVALUE; }
<ATTRCTX>"type"        { attrArg(); return token::ATTRTYPE; }
<ATTRCTX>"cname"       { attrArg(ShortString); return token::ATTRCNAME; }
<ATTRCTX>"order"       { attrArg(); return token::ATTRORDER; }
<ATTRCTX>"single"      { attrArg(); return token::ATTRSINGLE; }
<ATTRCTX>"tokenizer"   { attrArg(FallbackString); return token::ATTRTOKENIZER; }
<ATTRCTX>","           { attrArg(); return YYText()[0]; }
<ATTRCTX>[a-zA-Z0-9_]+ { yylval->emplace<std::string>(YYText()); return token::INVALID_ATTR; }
<ATTRCTX>"]"           { BEGIN(INITIAL); return YYText()[0]; }

<ATTRARGCTX,ATTRSHORTARGCTX,ATTRFALLBACKARGCTX>","               { return YYText()[0]; }
<ATTRARGCTX,ATTRFALLBACKARGCTX>" "                               { }
<ATTRARGCTX,ATTRFALLBACKARGCTX>true|false                        { yylval->emplace<bool>(YYText()[0] == 't'); return token::BOOL; }
<ATTRARGCTX>[a-z_]{SYMBOL}*                                      { yylval->emplace<std::string>(YYText()); return token::INVALID_ARG; }
<ATTRFALLBACKARGCTX>{FLOAT}                                      { yylval->emplace<double>(std::stof(YYText())); return token::FLOAT; }
<ATTRFALLBACKARGCTX>{INT}                                        { yylval->emplace<int64_t>(std::stoll(YYText())); return token::INT; }
<ATTRSHORTARGCTX,ATTRFALLBACKARGCTX>{SYMBOL}+                    { yylval->emplace<std::string>(YYText()); return token::STR; }
<ATTRSHORTARGCTX,ATTRFALLBACKARGCTX>{SYMBOL}+{SYMBOLS}*{SYMBOL}+ { yylval->emplace<std::string>(YYText()); return token::INVALID_ARG; }
<ATTRARGCTX,ATTRSHORTARGCTX,ATTRFALLBACKARGCTX>")"               { BEGIN(ATTRCTX); return YYText()[0]; }

"@"                { BEGIN(DOCCTX); return isDeclaring() ? token::IDOC : token::DOC; }
<DOCCTX>{DOCCHAR}+ { yylval->emplace<std::string>(unescape(trim(YYText()))); return token::STR; }
<DOCCTX>"{"        { BEGIN(REFCTX); return YYText()[0]; }
<DOCCTX>\[         { BEGIN(INITIAL); yyless(yyleng - 1); }
<DOCCTX>\r?\n      { yylloc->lines(); setDeclaring(false); BEGIN(INITIAL); }
<REFCTX>"}"        { BEGIN(DOCCTX); return YYText()[0]; }

import[ ]+ { BEGIN(IMPORTCTX); }
<IMPORTCTX>.*\r?\n {
    std::string importName = YYText();
    std::string lastString = "";
    if (auto firstSpace = importName.find_first_of(" @[:"); firstSpace != std::string::npos) {
        lastString = importName.substr(firstSpace + 1);
        importName = importName.substr(0, firstSpace);
    }
    yylloc->lines();
    import(*yylloc, importName);
    BEGIN(INITIAL);
    unput('\n');
    unput('\n');
    for (auto it = lastString.rbegin(); it != lastString.rend(); ++it) {
        unput(*it);
    }
    unput(' ');
    for (auto it = importName.rbegin(); it != importName.rend(); ++it) {
        unput(*it);
    }
    setDeclaring();
    return token::IMPORT;
}

"//".* ;

":" { return YYText()[0]; }
"," { return YYText()[0]; }
"{" { return YYText()[0]; }
"}" { return YYText()[0]; }

<*>[A-Z][a-zA-Z0-9]*   { yylval->emplace<std::string>(YYText()); return token::ID; }
<*>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<*>\"(\\.|[^\\"\n])*\" { std::string str = YYText(); str = str.substr(1, str.length() - 2); yylval->emplace<std::string>(str); return token::STR; }
<*>\"(\\.|[^\\"\n])*   { std::string str = YYText(); log(E3009, str.substr(0, str.length() - 1)); }
<*>{FLOAT}             { yylval->emplace<double>(std::stof(YYText())); return token::FLOAT; }
<*>{INT}               { yylval->emplace<int64_t>(std::stoll(YYText())); return token::INT; }
<*>true|false          { yylval->emplace<bool>(YYText()[0] == 't'); return token::BOOL; }

<*><<EOF>>                { setDeclaring(false); if (popImport()) { return token::POPIMPORT; } else { return token::YYEOF; } }
<*>\r?\n                  { yylloc->lines(); setDeclaring(false); }
<*>\t                     { log(E3020); }
<*>" "                    { }
<*>[A-Za-z0-9]+           { log(E3001, YYText()); }
<*>.                      { log(E3001, YYText()); }

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
