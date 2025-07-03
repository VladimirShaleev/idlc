%{
#include <sstream>
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
%x ATTRARGCNAME
%x ATTRARGARRAY
%x ATTRARGDATASIZE
%x ATTRARGTOKENIZER
%x ATTRARGVERSION
%x TYPE
%x DECLREF
%x IMPORT

DOCCHAR  ([^ \r\n\t\{\}[\]]|\\\{|\\\}|\\\[|\\\])
DOCMCHAR ([^ \r\n\t\{\}[\]`]|^[`]{3}|\\\{|\\\}|\\\[|\\\])

%%

%{
    yylloc->step();
%}

"api"       { context().setDeclaring(); return token::API; }
"enum"      { context().setDeclaring(); return token::ENUM; }
"const"     { context().setDeclaring(); return token::CONST; }
"struct"    { context().setDeclaring(); return token::STRUCT; }
"field"     { context().setDeclaring(); return token::FIELD; }
"interface" { context().setDeclaring(); return token::INTERFACE; }
"method"    { context().setDeclaring(); return token::METHOD; }
"arg"       { context().setDeclaring(); return token::ARG; }
"prop"      { context().setDeclaring(); return token::PROP; }
"event"     { context().setDeclaring(); return token::EVENT; }
"handle"    { context().setDeclaring(); return token::HANDLE; }
"func"      { context().setDeclaring(); return token::FUNC; }
"callback"  { context().setDeclaring(); return token::CALLBACK; }

"@"                        { BEGIN(DOCSTR); return context().isDeclaring() ? token::IDOC : token::DOC; }
<DOCSTR>{DOCCHAR}+         { yylval->emplace<std::string>(unescape(YYText())); return token::STR; }
<DOCSTR>"[brief]"[ ]*$     { return token::DOCBRIEF; }
<DOCSTR>"[detail]"[ ]*$    { return token::DOCDETAIL; }
<DOCSTR>"[author]"[ ]*$    { return token::DOCAUTHOR; }
<DOCSTR>"[see]"[ ]*$       { return token::DOCSEE; }
<DOCSTR>"[note]"[ ]*$      { return token::DOCNOTE; }
<DOCSTR>"[warning]"[ ]*$   { return token::DOCWARN; }
<DOCSTR>"[copyright]"[ ]*$ { return token::DOCCOPYRIGHT; }
<DOCSTR>"[license]"[ ]*$   { return token::DOCLICENSE; }
<DOCSTR>"[return]"[ ]*$    { return token::DOCRETURN; }
<DOCSTR>"[".*"]"[ ]*$      { err<IDL_STATUS_E2020>(*yylloc, YYText()); }
<DOCSTR>[\{\}]             { return YYText()[0]; }
<DOCSTR>\n                 { yylloc->lines(); BEGIN(INITIAL); }
<DOCSTR>\t                 { err<IDL_STATUS_E2002>(*yylloc); }
<DOCSTR>[ ]+               { yylval->emplace<std::string>(" "); return token::STR; }
<DOCSTR>.                  { err<IDL_STATUS_E2001>(*yylloc, YYText()); }
<DOCSTR>"```\n"            { yylloc->lines(); BEGIN(DOCMSTR); }

<DOCMSTR>"`"            { yylloc->lines(); yylval->emplace<std::string>(YYText()); return token::STR; }
<DOCMSTR>{DOCMCHAR}+    { yylval->emplace<std::string>(unescape(YYText())); return token::STR; }
<DOCMSTR>^[ ]+          {
    if (lineIndent < 0) {
        lineIndent = yyleng;
    } else {
        const auto spaces = yyleng - lineIndent;
        if (spaces > 0) {
            yylval->emplace<std::string>(fmt::format("{:<{}}", ' ', spaces));
            return token::STR;
        }
    }
}
<DOCMSTR>[\{\}]         { return YYText()[0]; }
<DOCMSTR>\n             { yylloc->lines(); yylval->emplace<std::string>(YYText()); return token::STR; }
<DOCMSTR>\t             { err<IDL_STATUS_E2002>(*yylloc); }
<DOCMSTR>[ ]+           { yylval->emplace<std::string>(YYText()); return token::STR; }
<DOCMSTR>.              { err<IDL_STATUS_E2001>(*yylloc, YYText()); }
<DOCMSTR>"```"          { BEGIN(DOCSTR); lineIndent = -1; }

"["                  { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRCTX>"flags"     { return token::ATTRFLAGS; }
<ATTRCTX>"hex"       { return token::ATTRHEX; }
<ATTRCTX>"platform"  { BEGIN(ATTRARGPATFORM); return token::ATTRPLATFORM; }
<ATTRCTX>"value"     { BEGIN(ATTRARGVALUE); return token::ATTRVALUE; }
<ATTRCTX>"type"      { BEGIN(ATTRARGTYPE); return token::ATTRTYPE; }
<ATTRCTX>"static"    { return token::ATTRSTATIC; }
<ATTRCTX>"ctor"      { return token::ATTRCTOR; }
<ATTRCTX>"this"      { return token::ATTRTHIS; }
<ATTRCTX>"get"       { BEGIN(ATTRARGTYPE); return token::ATTRGET; }
<ATTRCTX>"set"       { BEGIN(ATTRARGTYPE); return token::ATTRSET; }
<ATTRCTX>"handle"    { return token::ATTRHANDLE; }
<ATTRCTX>"cname"     { BEGIN(ATTRARGCNAME); return token::ATTRCNAME; }
<ATTRCTX>"array"     { BEGIN(ATTRARGARRAY); return token::ATTRARRAY; }
<ATTRCTX>"datasize"  { BEGIN(ATTRARGDATASIZE); return token::ATTRDATASIZE; }
<ATTRCTX>"const"     { return token::ATTRCONST; }
<ATTRCTX>"ref"       { return token::ATTRREF; }
<ATTRCTX>"refinc"    { return token::ATTRREFINC; }
<ATTRCTX>"userdata"  { return token::ATTRUSERDATA; }
<ATTRCTX>"errorcode" { return token::ATTRERRORCODE; }
<ATTRCTX>"noerror"   { return token::ATTRNOERROR; }
<ATTRCTX>"result"    { return token::ATTRRESULT; }
<ATTRCTX>"destroy"   { return token::ATTRDESTROY; }
<ATTRCTX>"in"        { return token::ATTRIN; }
<ATTRCTX>"out"       { return token::ATTROUT; }
<ATTRCTX>"optional"  { return token::ATTROPTIONAL; }
<ATTRCTX>"tokenizer" { BEGIN(ATTRARGTOKENIZER); return token::ATTRTOKENIZER; }
<ATTRCTX>"version"   { BEGIN(ATTRARGVERSION); return token::ATTRVERSION; }
<ATTRCTX>","         { return YYText()[0]; }
<ATTRCTX>" "         ;
<ATTRCTX>\n          { yylloc->lines(); }
<ATTRCTX>[a-z]+      { err<IDL_STATUS_E2015>(*yylloc, YYText()); }
<ATTRCTX>[^\]\(]     { err<IDL_STATUS_E2001>(*yylloc, YYText()); }
<ATTRCTX>"]"         { BEGIN(INITIAL); return YYText()[0]; }

<ATTRARGVALUE>"("                 { return YYText()[0]; }
<ATTRARGVALUE>")"                 { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGVALUE>[-+]?[0-9]+         { yylval->emplace<int64_t>(std::stoll(YYText())); return token::NUM; }
<ATTRARGVALUE>"true"              { yylval->emplace<bool>(true); return token::BOOL; }
<ATTRARGVALUE>"false"             { yylval->emplace<bool>(false); return token::BOOL; }
<ATTRARGVALUE>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<ATTRARGVALUE>","                 { return YYText()[0]; }
<ATTRARGVALUE>" "                 ;
<ATTRARGVALUE>\n                  { yylloc->lines(); }
<ATTRARGVALUE>.                   { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

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
<ATTRARGPATFORM>.         { err<IDL_STATUS_E2017>(*yylloc, "'windows', 'linux', 'macos', 'web', 'android', 'ios"); }

<ATTRARGTYPE>"("                 { return YYText()[0]; }
<ATTRARGTYPE>")"                 { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGTYPE>" "                 ;
<ATTRARGTYPE>\n                  { yylloc->lines(); }
<ATTRARGTYPE>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<ATTRARGTYPE>.                   { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

<ATTRARGCNAME>"("           { return YYText()[0]; }
<ATTRARGCNAME>")"           { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGCNAME>" "           ;
<ATTRARGCNAME>\n            { yylloc->lines(); }
<ATTRARGCNAME>[_a-zA-Z0-9]+ { yylval->emplace<std::string>(YYText()); return token::STR; }
<ATTRARGCNAME>.             { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

<ATTRARGARRAY>"("                 { return YYText()[0]; }
<ATTRARGARRAY>")"                 { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGARRAY>" "                 ;
<ATTRARGARRAY>\n                  { yylloc->lines(); }
<ATTRARGARRAY>[0-9]+              { yylval->emplace<int64_t>(std::stoi(YYText())); return token::NUM; }
<ATTRARGARRAY>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<ATTRARGARRAY>.                   { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

<ATTRARGDATASIZE>"("                 { return YYText()[0]; }
<ATTRARGDATASIZE>")"                 { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGDATASIZE>" "                 ;
<ATTRARGDATASIZE>\n                  { yylloc->lines(); }
<ATTRARGDATASIZE>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<ATTRARGDATASIZE>.                   { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

<ATTRARGTOKENIZER>"("                    { return YYText()[0]; }
<ATTRARGTOKENIZER>")"                    { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGTOKENIZER>" "                    ;
<ATTRARGTOKENIZER>\n                     { yylloc->lines(); }
<ATTRARGTOKENIZER>\^?[0-9]+(-\^?[0-9]+)* { yylval->emplace<std::string>(YYText()); return token::TOKINDX; }
<ATTRARGTOKENIZER>.                      { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

<ATTRARGVERSION>"("    { return YYText()[0]; }
<ATTRARGVERSION>")"    { BEGIN(ATTRCTX); return YYText()[0]; }
<ATTRARGVERSION>","    { return YYText()[0]; }
<ATTRARGVERSION>" "    ;
<ATTRARGVERSION>\n     { yylloc->lines(); }
<ATTRARGVERSION>[0-9]+ { yylval->emplace<int>(std::stoi(YYText())); return token::NUM; }
<ATTRARGVERSION>.      { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

"b166074c3cba4005a198513772597880" { context().setDeclaring(); return token::FILEDOC; }
import[ ]+ { BEGIN(IMPORT); }
<IMPORT>[-\.a-zA-Z0-9_]+ {
    std::string importName = YYText();
    int c;
    while ((c = yyinput()) && c != '\n') {
        if (c != ' ') {
            err<IDL_STATUS_E2001>(*yylloc, YYText());
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
<IMPORT>.|\n { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

<TYPE>" "                 ;
<TYPE>\n                  { yylloc->lines(); }
<TYPE>"}"                 { BEGIN(INITIAL); return YYText()[0]; }
<TYPE>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<TYPE>.                   { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

<DECLREF>" "                 ;
<DECLREF>","                 { return YYText()[0]; }
<DECLREF>\n                  { yylloc->lines(); }
<DECLREF>[A-Z][a-zA-Z0-9\.]* { yylval->emplace<std::string>(YYText()); return token::REF; }
<DECLREF>.                   { BEGIN(INITIAL); unput(YYText()[0]); }

[A-Z][a-zA-Z0-9]*         { yylval->emplace<std::string>(YYText()); return token::ID; }
[-+]?[0-9]+               { yylval->emplace<int64_t>(std::stoll(YYText())); return token::NUM; }
"true"                    { yylval->emplace<bool>(true); return token::BOOL; }
"false"                   { yylval->emplace<bool>(false); return token::BOOL; }
[a-zA-Z0-9]+              { err<IDL_STATUS_E2003>(*yylloc, YYText()); }
<<EOF>>                   { context().setDeclaring(false); if (!popImport()) { return token::YYEOF; } }
\n                        { yylloc->lines(); context().setDeclaring(false); }
\t                        { err<IDL_STATUS_E2002>(*yylloc); }
" "                       ;
":"                       { BEGIN(DECLREF); return YYText()[0]; }
"{"                       { BEGIN(TYPE); return YYText()[0]; }
"//".*                    ;
.                         { err<IDL_STATUS_E2001>(*yylloc, YYText()); }

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
