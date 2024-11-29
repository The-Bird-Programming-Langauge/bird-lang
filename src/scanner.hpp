#ifndef SCANNER_HPP
#define SCANNER_HPP

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL Bird::Parser::token_kind_type Bird::Scanner::next_token(Bird::Parser::semantic_type *value, Bird::Parser::location_type *location)

#include "../build/parser.tab.hh"
#include "../build/location.hh"

#include <vector>
namespace Bird
{
    class Scanner : public yyFlexLexer
    {
    private:
        Bird::Parser::semantic_type yylval;

    public:
        Scanner(std::istream *in) : yyFlexLexer(in) {}

        virtual ~Scanner() {}

        virtual Bird::Parser::token_kind_type next_token(Bird::Parser::semantic_type *value, Bird::Parser::location_type *location);
    };
}

#endif