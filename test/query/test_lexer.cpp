#include <gtest/gtest.h>

#include "currentia/query/lexer.h"

using namespace currentia;

TEST (testPrimitive, NUMBER) {
    std::istringstream is("12345 3.14");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_INTEGER, lexer.get_next_token());
    EXPECT_EQ("12345", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_FLOAT, lexer.get_next_token());
    EXPECT_EQ("3.14", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (testLexer, empty) {
    std::istringstream is("");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (testLexer, whitespace) {
    std::istringstream is("                  ");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (testLexer, comment) {
    std::istringstream is("SELECT # SELECT FROM WHERE");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_SELECT, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (testLexer, keywords) {
    std::istringstream is("SELECT FROM WHERE");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_SELECT, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_FROM, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_WHERE, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (testLexer, keywordsIgnoreCase) {
    std::istringstream is("select frOM wHere");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_SELECT, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_FROM, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_WHERE, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (testLexer, name) {
    std::istringstream is("foo bar baz");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("foo", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("bar", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("baz", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (testLexer, DDL) {
    std::istringstream is("CREATE STREAM members (name STRING, age INT, income INT)");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_CREATE, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_STREAM, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("members", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_LPAREN, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("name", lexer.get_token_text());
    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("STRING", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_COMMA, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("age", lexer.get_token_text());
    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("INT", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_COMMA, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("income", lexer.get_token_text());
    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("INT", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_RPAREN, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (testLexer, IN) {
    std::istringstream is("WHERE X IN Y");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_WHERE, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("X", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_IN, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("Y", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}

TEST (TestDDL, TABLE) {
    std::istringstream is("CREATE TABLE members (name STRING, age INT, income INT)");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::TOKEN_CREATE, lexer.get_next_token());
    EXPECT_EQ(Lexer::TOKEN_TABLE, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("members", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_LPAREN, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("name", lexer.get_token_text());
    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("STRING", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_COMMA, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("age", lexer.get_token_text());
    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("INT", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_COMMA, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("income", lexer.get_token_text());
    EXPECT_EQ(Lexer::TOKEN_NAME, lexer.get_next_token());
    EXPECT_EQ("INT", lexer.get_token_text());

    EXPECT_EQ(Lexer::TOKEN_RPAREN, lexer.get_next_token());

    EXPECT_EQ(Lexer::TOKEN_EOS, lexer.get_next_token());
}
