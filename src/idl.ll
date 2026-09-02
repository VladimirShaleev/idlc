%{
#include <sstream>
#include "scanner.hpp"
#define YY_NO_UNISTD_H
#define YY_DECL int idl::Scanner::yylex(idl::Parser::semantic_type* yylval, idl::Parser::location_type* yylloc)
#define YY_USER_ACTION action(*yylloc);
#define ast_location { \
    context().result()->intern({yylloc->begin.filename->c_str(), yylloc->begin.filename->length()}), \
    uint16_t(yylloc->begin.column), \
    uint16_t(yylloc->begin.line) }
#define log(status, ...) context().log<IDL_STATUS_##status>(ast_location __VA_OPT__(,) __VA_ARGS__)
#define yy_text_view std::string_view { YYText(), size_t(YYLeng()) } 
#define parse_float(str) parseFloat(ast_location, str)
#define parse_int(str) parseInt(ast_location, str)
using namespace std::string_literals;
typedef idl::Parser::token token;
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
%x MDOCCTX
%x REFCTX
%x IMPORTCTX

MDOCCHAR ([^ \r\n\t\{\}[\]\\`]|\\\{|\\\}|\\\[|\\\]|\\\\|\\`)
DOCCHAR  ([^ \r\n\t\{\}[\]\\]|\\\{|\\\}|\\\[|\\\]|\\\\)
FLOAT    [-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
INT      [-+]?[0-9]+
SYMBOL   [a-zA-Z0-9_\-^\.@:]
SYMBOLS  [a-zA-Z0-9_\-^\.@: ]

%%

%{
    yylloc->step();
%}

"api"    { setDeclaring(); return token::API; }
"enum"   { setDeclaring(); return token::ENUM; }
"const"  { setDeclaring(); return token::CONST; }
"struct" { setDeclaring(); return token::STRUCT; }
"field"  { setDeclaring(); return token::FIELD; }
"func"   { setDeclaring(); return token::FUNC; }
"arg"    { setDeclaring(); return token::ARG; }

"["                    { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRCTX>"("           { getAttrArg() == Default ? BEGIN(ATTRARGCTX) : getAttrArg() == ShortString ? BEGIN(ATTRSHORTARGCTX) : BEGIN(ATTRFALLBACKARGCTX); return YYText()[0]; }
<ATTRCTX>"version"     { attrArg(FallbackString); return token::ATTRVERSION; }
<ATTRCTX>"author"      { attrArg(); return token::ATTRAUTHOR; }
<ATTRCTX>"copyright"   { attrArg(); return token::ATTRCOPYRIGHT; }
<ATTRCTX>"license"     { attrArg(); return token::ATTRLICENSE; }
<ATTRCTX>"flags"       { attrArg(); return token::ATTRFLAGS; }
<ATTRCTX>"hex"         { attrArg(); return token::ATTRHEX; }
<ATTRCTX>"maxenum"     { attrArg(); return token::ATTRMAXENUM; }
<ATTRCTX>"countenums"  { attrArg(); return token::ATTRCOUNTENUMS; }
<ATTRCTX>"typedenums"  { attrArg(); return token::ATTRTYPEDENUMS; }
<ATTRCTX>"brief"       { attrArg(); return token::ATTRBRIEF; }
<ATTRCTX>"detail"      { attrArg(); return token::ATTRDETAIL; }
<ATTRCTX>"return"      { attrArg(); return token::ATTRRETURN; }
<ATTRCTX>"value"       { attrArg(); return token::ATTRVALUE; }
<ATTRCTX>"type"        { attrArg(); return token::ATTRTYPE; }
<ATTRCTX>"cname"       { attrArg(ShortString); return token::ATTRCNAME; }
<ATTRCTX>"cconv"       { attrArg(ShortString); return token::ATTRCCONV; }
<ATTRCTX>"cformat"     { attrArg(ShortString); return token::ATTRCFORMAT; }
<ATTRCTX>"single"      { attrArg(); return token::ATTRSINGLE; }
<ATTRCTX>"stdtypes"    { attrArg(); return token::ATTRSTDTYPES; }
<ATTRCTX>"booltype"    { attrArg(); return token::ATTRBOOLTYPE; }
<ATTRCTX>"tokenizer"   { attrArg(FallbackString); return token::ATTRTOKENIZER; }
<ATTRCTX>"array"       { attrArg(FallbackString); return token::ATTRARRAY; }
<ATTRCTX>"ref"         { attrArg(); return token::ATTRREF; }
<ATTRCTX>"const"       { attrArg(); return token::ATTRCONST; }
<ATTRCTX>"optional"    { attrArg(); return token::ATTROPTIONAL; }
<ATTRCTX>","           { attrArg(); return YYText()[0]; }
<ATTRCTX>[a-zA-Z0-9_]+ { yylval->emplace<std::string>(YYText()); return token::INVALID_ATTR; }
<ATTRCTX>"]"           { BEGIN(INITIAL); return YYText()[0]; }

<ATTRARGCTX,ATTRSHORTARGCTX,ATTRFALLBACKARGCTX>","               { return YYText()[0]; }
<ATTRARGCTX,ATTRFALLBACKARGCTX>" "                               { }
<ATTRARGCTX,ATTRFALLBACKARGCTX>true|false                        { yylval->emplace<bool>(YYText()[0] == 't'); return token::BOOL; }
<ATTRARGCTX>[a-z_]{SYMBOL}*                                      { yylval->emplace<std::string>(YYText()); return token::INVALID_ARG; }
<ATTRFALLBACKARGCTX>{FLOAT}                                      { yylval->emplace<double>(parse_float(YYText())); return token::FLOAT; }
<ATTRFALLBACKARGCTX>{INT}                                        { yylval->emplace<int64_t>(parse_int(YYText())); return token::INT; }
<ATTRSHORTARGCTX,ATTRFALLBACKARGCTX>{SYMBOL}+                    { yylval->emplace<std::string>(YYText()); return token::STR; }
<ATTRSHORTARGCTX,ATTRFALLBACKARGCTX>{SYMBOL}+{SYMBOLS}*{SYMBOL}+ { yylval->emplace<std::string>(YYText()); return token::INVALID_ARG; }
<ATTRARGCTX,ATTRSHORTARGCTX,ATTRFALLBACKARGCTX>")"               { BEGIN(ATTRCTX); return YYText()[0]; }

@[ ]*```[ ]*\r?\n         { yylloc->lines(); BEGIN(MDOCCTX); setMultiline(true); return isDeclaring() ? token::IMDOC : token::MDOC; }
@[ ]*```                  { log(E3017); BEGIN(MDOCCTX); setMultiline(true); return isDeclaring() ? token::IMDOC : token::MDOC; }
<MDOCCTX>[ ]*```[ ]*\[    { setDeclaring(false); BEGIN(INITIAL); yyless(yyleng - 1); }
<MDOCCTX>[ ]*```[ ]*\r?\n { yylloc->lines(); setDeclaring(false); BEGIN(INITIAL); }
<MDOCCTX>\r?\n            { yylloc->lines(); yylval->emplace<std::string>("\n"); return token::STR; }
<MDOCCTX>[ ]+\r?\n        { yylloc->lines(); yylval->emplace<std::string>("\n"); return token::STR; }
<MDOCCTX>\t               { yylval->emplace<std::string>("\t"); return token::STR; }
<MDOCCTX>\\\\             { yylval->emplace<std::string>("\\"); return token::STR; }
<MDOCCTX>\\*              { log(W2005, "'\\\\', '\\{', '\\}', '\\[', '\\]', '\\`'"); yylval->emplace<std::string>(YYText()); return token::STR; }
<MDOCCTX>[ ]+             { yylval->emplace<std::string>(YYText()); return token::STR; }
<MDOCCTX>{MDOCCHAR}+      { yylval->emplace<std::string>(unescape(yy_text_view, true, *yylloc)); return token::STR; }
<MDOCCTX>"{"              { BEGIN(REFCTX); return YYText()[0]; } 

@[ ]*              { BEGIN(DOCCTX); setMultiline(false); return isDeclaring() ? token::IDOC : token::DOC; }
<DOCCTX>\r?\n      { yylloc->lines(); setDeclaring(false); BEGIN(INITIAL); }
<DOCCTX>[ ]+\r?\n  { yylloc->lines(); setDeclaring(false); BEGIN(INITIAL); }
<DOCCTX>[ ]+       { yylval->emplace<std::string>(" "); return token::STR; }
<DOCCTX>(\\s)+     { yylval->emplace<std::string>(fmt::format("{:{}}", " ", YYLeng() / 2)); return token::STR; }
<DOCCTX>(\\t)+     { yylval->emplace<std::string>(fmt::format("{:{}}", "\t", YYLeng() / 2)); return token::STR; }
<DOCCTX>\\n        { yylval->emplace<std::string>("\n"); return token::STR; }
<DOCCTX>\\\\       { yylval->emplace<std::string>("\\"); return token::STR; }
<DOCCTX>\\*        { log(W2005, "'\\\\', '\\{', '\\}', '\\[', '\\]', '\\s', '\\t', '\\n'"); yylval->emplace<std::string>(YYText()); return token::STR; }
<DOCCTX>{DOCCHAR}+ { yylval->emplace<std::string>(unescape(yy_text_view, false, *yylloc)); return token::STR; }
<DOCCTX>"{"        { BEGIN(REFCTX); return YYText()[0]; }
<DOCCTX>[ ]*\[     { BEGIN(INITIAL); yyless(yyleng - 1); }
<REFCTX>"}"        { if (isMultiline()) { BEGIN(MDOCCTX); } else { BEGIN(DOCCTX); } return YYText()[0]; }

import[ ]+ { BEGIN(IMPORTCTX); }
<IMPORTCTX>.*\r?\n {
    std::string importName = YYText();
    std::string lastString = "";
    if (auto firstSpace = importName.find_first_of(" @[:"); firstSpace != std::string::npos) {
        lastString = importName.substr(firstSpace + 1);
        importName = importName.substr(0, firstSpace);
    }
    trim(importName);
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

<*>[A-Z][a-zA-Z0-9]*     { yylval->emplace<std::string>(YYText()); return token::ID; }
<*>[A-Z][a-zA-Z0-9\.]*   { yylval->emplace<std::string>(YYText()); return token::REF; }
<*>true|false            { yylval->emplace<bool>(YYText()[0] == 't'); return token::BOOL; }
<*>{FLOAT}               { yylval->emplace<double>(parse_float(YYText())); return token::FLOAT; }
<*>{INT}                 { yylval->emplace<int64_t>(parse_int(YYText())); return token::INT; }
<*>{SYMBOL}+             { yylval->emplace<std::string>(YYText()); return token::INVALID_ID; }
<*>\"(\\.|[^\\"\r\n])*\" { std::string str = YYText(); str = str.substr(1, str.length() - 2); yylval->emplace<std::string>(cunescape(str, *yylloc)); return token::STR; }
<*>\"(\\.|[^\\"\r\n])*   { std::string str = YYText(); log(E3009, str.substr(0, str.length())); }

<*><<EOF>>                { setDeclaring(false); if (popImport()) { return token::POPIMPORT; } else { return token::YYEOF; } }
<*>\r?\n                  { yylloc->lines(); setDeclaring(false); }
<*>\t                     { log(E3020); }
<*>" "                    { }
<*>.                      { log(E3001, YYText()); }

%%

int yyFlexLexer::yylex() {
    throw std::runtime_error("Bad call to yyFlexLexer::yylex()");
}
