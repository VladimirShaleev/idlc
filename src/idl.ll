%{
#include <string>
#include "scanner.hpp"
#define YY_NO_UNISTD_H
#define YY_DECL int idl::Scanner::yylex(idl::Parser::semantic_type* yylval, idl::Parser::location_type* yylloc)
#define YY_USER_ACTION yylloc->step(); yylloc->columns(yyleng);
typedef idl::Parser::token token;
%}

%option c++
%option noyywrap
%option yylineno
%option nodefault
%option outfile="scanner.cpp"

%%

%{
    yylloc->step();
%}

"namespace" return token::NAMESPACE;
"enum"      return token::ENUM;
"interface" return token::INTERFACE;

[a-zA-Z_][a-zA-Z0-9_]* { yylval->emplace<std::string>(YYText()); return token::ID; }
[-+]?[0-9]+            { char* pEnd; yylval->emplace<int64_t>(std::strtoll(YYText(), &pEnd, 10)); return token::NUM; }
[-+]?0x[0-9a-fA-F]+    { char* pEnd; yylval->emplace<int64_t>(std::strtoll(YYText(), &pEnd, 16)); return token::NUM; }
[\{\}=\.,;()[\]]       { return YYText()[0]; }
\n                     { yylloc->lines(); }
[ \t]                  ;
<<EOF>>                { return token::YYEOF; }
.                      { std::cerr << "error: unexpected character '" << YYText() << "' at " << Location(*yylloc) << std::endl; yyterminate(); }

%%

int yyFlexLexer::yylex() {
    throw std::runtime_error("Bad call to yyFlexLexer::yylex()");
}
