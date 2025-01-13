#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(LexerTest, TokenizeBlockStmt)
{
    BirdTest::TestOptions options;
    options.code = "{ var x: int = 10; var y: int = 20;}";

    options.compile = false;
    options.interpret = false;
    options.parse = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 17);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::LBRACE);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::VAR);
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[2].lexeme, "x");
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::COLON);
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::TYPE_LITERAL);
    //     ASSERT_EQ(lexer.tokens[4].lexeme, "int");
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::EQUAL);
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[6].lexeme, "10");
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[8].token_type, Token::Type::VAR);
    //     ASSERT_EQ(lexer.tokens[9].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[9].lexeme, "y");
    //     ASSERT_EQ(lexer.tokens[10].token_type, Token::Type::COLON);
    //     ASSERT_EQ(lexer.tokens[11].token_type, Token::Type::TYPE_LITERAL);
    //     ASSERT_EQ(lexer.tokens[11].lexeme, "int");
    //     ASSERT_EQ(lexer.tokens[12].token_type, Token::Type::EQUAL);
    //     ASSERT_EQ(lexer.tokens[13].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[13].lexeme, "20");
    //     ASSERT_EQ(lexer.tokens[14].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[15].token_type, Token::Type::RBRACE);
    //     ASSERT_EQ(lexer.tokens[16].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeConstStmt)
{
    BirdTest::TestOptions options;
    options.code = "const z: str = \"foobar\";";

    options.compile = false;
    options.interpret = false;
    options.parse = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 8);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::CONST);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[1].lexeme, "z");
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::COLON);
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::TYPE_LITERAL);
    //     ASSERT_EQ(lexer.tokens[3].lexeme, "str");
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::EQUAL);
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::STR_LITERAL);
    //     ASSERT_EQ(lexer.tokens[5].lexeme, "foobar");
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeDeclStmt)
{
    BirdTest::TestOptions options;
    options.code = "var x: int = 3;";

    options.compile = false;
    options.interpret = false;
    options.parse = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 8);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::VAR);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[1].lexeme, "x");
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::COLON);
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::TYPE_LITERAL);
    //     ASSERT_EQ(lexer.tokens[3].lexeme, "int");
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::EQUAL);
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[5].lexeme, "3");
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeExprStmt)
{
    BirdTest::TestOptions options;
    options.code = "1 + 2 - 3 / 6 * 4 <= 4";

    options.compile = false;
    options.interpret = false;
    options.parse = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 12);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[0].lexeme, "1");
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::PLUS);
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[2].lexeme, "2");
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::MINUS);
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[4].lexeme, "3");
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::SLASH);
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[6].lexeme, "6");
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::STAR);
    //     ASSERT_EQ(lexer.tokens[8].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[8].lexeme, "4");
    //     ASSERT_EQ(lexer.tokens[9].token_type, Token::Type::LESS_EQUAL);
    //     ASSERT_EQ(lexer.tokens[10].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[10].lexeme, "4");
    //     ASSERT_EQ(lexer.tokens[11].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeFuncStmt)
{
    BirdTest::TestOptions options;
    options.code = "fn add(first: int, second: int) -> int { return first + second; }";

    options.compile = false;
    options.interpret = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 21);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::FN);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[1].lexeme, "add");
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::LPAREN);
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[3].lexeme, "first");
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::COLON);
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::TYPE_LITERAL);
    //     ASSERT_EQ(lexer.tokens[5].lexeme, "int");
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::COMMA);
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[7].lexeme, "second");
    //     ASSERT_EQ(lexer.tokens[8].token_type, Token::Type::COLON);
    //     ASSERT_EQ(lexer.tokens[9].token_type, Token::Type::TYPE_LITERAL);
    //     ASSERT_EQ(lexer.tokens[9].lexeme, "int");
    //     ASSERT_EQ(lexer.tokens[10].token_type, Token::Type::RPAREN);
    //     ASSERT_EQ(lexer.tokens[11].token_type, Token::Type::ARROW);
    //     ASSERT_EQ(lexer.tokens[12].token_type, Token::Type::TYPE_LITERAL);
    //     ASSERT_EQ(lexer.tokens[12].lexeme, "int");
    //     ASSERT_EQ(lexer.tokens[13].token_type, Token::Type::LBRACE);
    //     ASSERT_EQ(lexer.tokens[14].token_type, Token::Type::RETURN);
    //     ASSERT_EQ(lexer.tokens[15].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[15].lexeme, "first");
    //     ASSERT_EQ(lexer.tokens[16].token_type, Token::Type::PLUS);
    //     ASSERT_EQ(lexer.tokens[17].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[17].lexeme, "second");
    //     ASSERT_EQ(lexer.tokens[18].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[19].token_type, Token::Type::RBRACE);
    //     ASSERT_EQ(lexer.tokens[20].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeIfStmt)
{
    BirdTest::TestOptions options;
    options.code = "if 1 > 2 {print 1;} else if 1 == 2 {print 2;} else {print 3;}";

    options.compile = false;
    options.interpret = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 20);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::IF);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[1].lexeme, "1");
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::GREATER);
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[3].lexeme, "2");
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::PRINT);
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[5].lexeme, "1");
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::ELSE);
    //     ASSERT_EQ(lexer.tokens[8].token_type, Token::Type::IF);
    //     ASSERT_EQ(lexer.tokens[9].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[9].lexeme, "1");
    //     ASSERT_EQ(lexer.tokens[10].token_type, Token::Type::EQUAL_EQUAL);
    //     ASSERT_EQ(lexer.tokens[11].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[11].lexeme, "2");
    //     ASSERT_EQ(lexer.tokens[12].token_type, Token::Type::PRINT);
    //     ASSERT_EQ(lexer.tokens[13].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[13].lexeme, "2");
    //     ASSERT_EQ(lexer.tokens[14].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[15].token_type, Token::Type::ELSE);
    //     ASSERT_EQ(lexer.tokens[16].token_type, Token::Type::PRINT);
    //     ASSERT_EQ(lexer.tokens[17].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[17].lexeme, "3");
    //     ASSERT_EQ(lexer.tokens[18].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[19].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizePrintStmt)
{
    BirdTest::TestOptions options;
    options.code = "print 28;";

    options.compile = false;
    options.interpret = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 4);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::PRINT);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[1].lexeme, "28");
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeWhileStmt)
{
    BirdTest::TestOptions options;
    options.code = "while (true) { print 1; }";

    options.compile = false;
    options.interpret = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 10);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::WHILE);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::LPAREN);
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::BOOL_LITERAL);
    //     ASSERT_EQ(lexer.tokens[2].lexeme, "true");
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::RPAREN);
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::LBRACE);
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::PRINT);
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[6].lexeme, "1");
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[8].token_type, Token::Type::RBRACE);
    //     ASSERT_EQ(lexer.tokens[9].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeBreakStmt)
{
    BirdTest::TestOptions options;
    options.code = "break;";

    options.compile = false;
    options.interpret = false;
    options.semantic_analyze = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 3);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::BREAK);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeContinueStmt)
{
    BirdTest::TestOptions options;
    options.code = "continue;";

    options.compile = false;
    options.interpret = false;
    options.semantic_analyze = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 3);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::CONTINUE);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeTernary)
{
    BirdTest::TestOptions options;
    options.code = "print 2.3 > 2.1 ? 1 : 0;";

    options.compile = false;
    options.interpret = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 10);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::PRINT);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::FLOAT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[1].lexeme, "2.3");
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::GREATER);
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::FLOAT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[3].lexeme, "2.1");
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::QUESTION);
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[5].lexeme, "1");
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::COLON);
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[7].lexeme, "0");
    //     ASSERT_EQ(lexer.tokens[8].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[9].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LexerTest, TokenizeForLoop)
{
    BirdTest::TestOptions options;
    options.code = "for var x: int = 0; x <= 5; x += 1 do print 1;";

    options.compile = false;
    options.interpret = false;

    // options.after_lex = [&](UserErrorTracker &error_tracker, Lexer &lexer)
    // {
    //     ASSERT_EQ(lexer.tokens.size(), 20);

    //     ASSERT_EQ(lexer.tokens[0].token_type, Token::Type::FOR);
    //     ASSERT_EQ(lexer.tokens[1].token_type, Token::Type::VAR);
    //     ASSERT_EQ(lexer.tokens[2].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[2].lexeme, "x");
    //     ASSERT_EQ(lexer.tokens[3].token_type, Token::Type::COLON);
    //     ASSERT_EQ(lexer.tokens[4].token_type, Token::Type::TYPE_LITERAL);
    //     ASSERT_EQ(lexer.tokens[4].lexeme, "int");
    //     ASSERT_EQ(lexer.tokens[5].token_type, Token::Type::EQUAL);
    //     ASSERT_EQ(lexer.tokens[6].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[6].lexeme, "0");
    //     ASSERT_EQ(lexer.tokens[7].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[8].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[8].lexeme, "x");
    //     ASSERT_EQ(lexer.tokens[9].token_type, Token::Type::LESS_EQUAL);
    //     ASSERT_EQ(lexer.tokens[10].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[10].lexeme, "5");
    //     ASSERT_EQ(lexer.tokens[11].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[12].token_type, Token::Type::IDENTIFIER);
    //     ASSERT_EQ(lexer.tokens[12].lexeme, "x");
    //     ASSERT_EQ(lexer.tokens[13].token_type, Token::Type::PLUS_EQUAL);
    //     ASSERT_EQ(lexer.tokens[14].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[14].lexeme, "1");
    //     ASSERT_EQ(lexer.tokens[15].token_type, Token::Type::DO);
    //     ASSERT_EQ(lexer.tokens[16].token_type, Token::Type::PRINT);
    //     ASSERT_EQ(lexer.tokens[17].token_type, Token::Type::INT_LITERAL);
    //     ASSERT_EQ(lexer.tokens[17].lexeme, "1");
    //     ASSERT_EQ(lexer.tokens[18].token_type, Token::Type::SEMICOLON);
    //     ASSERT_EQ(lexer.tokens[19].token_type, Token::Type::END);
    // };

    ASSERT_TRUE(BirdTest::compile(options));
}
