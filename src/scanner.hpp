#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "ast.hpp"
#include "parser.hpp"

#if !defined(yyFlexLexerOnce)
# include <FlexLexer.h>
#endif

namespace idl {

class Scanner : public yyFlexLexer {
public:
    Scanner(std::istream* in = nullptr, const std::string* filename = nullptr) : yyFlexLexer(in), _filename(filename) {
    }

    int yylex(Parser::semantic_type* yylval, Parser::location_type* yylloc);

    const std::string* filename() const noexcept {
        return _filename;
    }

    ASTProgram* program{};

private:
    const std::string* _filename{};
};

} // namespace idl

#endif
