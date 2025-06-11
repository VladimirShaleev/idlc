%{
#include "errors.hpp"
#include "scanner.hpp"
#define YY_NO_UNISTD_H
#define YY_DECL int idl::Scanner::yylex(idl::Parser::semantic_type* yylval, idl::Parser::location_type* yylloc)
#define YY_USER_ACTION yylloc->step(); yylloc->columns(yyleng);
using namespace std::string_literals;
typedef idl::Parser::token token;
%}

%option c++
%option noyywrap
%option yylineno
%option nodefault
%option outfile="scanner.cpp"

%x DOCSTR
%x DOCMSTR

%%

%{
    yylloc->step();
%}

"api" { return token::API; }

"@ "                                 { BEGIN(DOCSTR); return token::DOC; }
"@"                                  { BEGIN(DOCSTR); warn<W1001>(*yylloc); return token::DOC; }
"@"[ ][ ]+                           { BEGIN(DOCSTR); warn<W1002>(*yylloc); return token::DOC; }
<DOCSTR>\n                           { yylloc->lines(); BEGIN(INITIAL); }
<DOCSTR>([^ \n\t\{\}[\]]|\\\{|\\\})+ { yylval->emplace<std::string>(YYText()); return token::STR; }
<DOCSTR>"[brief]"[ ]*$               { return token::DOCBRIEF; }
<DOCSTR>"[detail]"[ ]*$              { return token::DOCDETAIL; }
<DOCSTR>"[author]"[ ]*$              { return token::DOCAUTHOR; }
<DOCSTR>"[copyright]"[ ]*$           { return token::DOCCOPYRIGHT; }
<DOCSTR>"[license]"[ ]*$             { return token::DOCLICENSE; }
<DOCSTR>[\{\}]                       { return YYText()[0]; }
<DOCSTR>\t                           { err<E2002>(*yylloc); yyterminate(); }
<DOCSTR>" "                          ;
<DOCSTR>.                            { err<E2001>(*yylloc, YYText()); yyterminate(); }
<DOCSTR>"```\n"                      { yylloc->lines(); BEGIN(DOCMSTR); }


<DOCMSTR>\n                                    { yylloc->lines(); yylval->emplace<std::string>(YYText()); return token::STR; }
<DOCMSTR>([^ \n\t\{\}[\]`]|^[`]{3}|\\\{|\\\})+ { yylval->emplace<std::string>(YYText()); return token::STR; }
<DOCMSTR>[\{\}]                                { return YYText()[0]; }
<DOCMSTR>^[ ]+                                 ;
<DOCMSTR>" "                                   ;
<DOCMSTR>\t                                    { err<E2002>(*yylloc); yyterminate(); }
<DOCMSTR>.                                     { err<E2001>(*yylloc, YYText()); yyterminate(); }
<DOCMSTR>"```"                                 { BEGIN(DOCSTR); }

[A-Z][a-zA-Z0-9]* { yylval->emplace<std::string>(YYText()); return token::ID; }
[a-zA-Z0-9]+      { err<E2003>(*yylloc, YYText()); yyterminate(); }
<<EOF>>           { return token::YYEOF; }
\n                { yylloc->lines(); }
\t                { err<E2002>(*yylloc); yyterminate(); }
" "               ;
"//".*            ;
.                 { err<E2001>(*yylloc, YYText()); yyterminate(); }

%%

int yyFlexLexer::yylex() {
    throw std::runtime_error("Bad call to yyFlexLexer::yylex()");
}
