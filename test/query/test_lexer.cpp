#include <gtest/gtest.h>

#include "currentia/query/lexer.h"

using namespace currentia;

TEST (testLexer, empty) {
    std::istringstream is("");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}

TEST (testLexer, whitespace) {
    std::istringstream is("                  ");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}

TEST (testLexer, comment) {
    std::istringstream is("SELECT # SELECT FROM WHERE");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::SELECT, lexer.next_token());
    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}

TEST (testLexer, keywords) {
    std::istringstream is("SELECT FROM WHERE");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::SELECT, lexer.next_token());
    EXPECT_EQ(Lexer::FROM, lexer.next_token());
    EXPECT_EQ(Lexer::WHERE, lexer.next_token());
    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}

TEST (testLexer, keywordsIgnoreCase) {
    std::istringstream is("select frOM wHere");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::SELECT, lexer.next_token());
    EXPECT_EQ(Lexer::FROM, lexer.next_token());
    EXPECT_EQ(Lexer::WHERE, lexer.next_token());
    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}

TEST (testLexer, name) {
    std::istringstream is("foo bar baz");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("FOO", lexer.get_latest_name());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("BAR", lexer.get_latest_name());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("BAZ", lexer.get_latest_name());

    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}

TEST (testLexer, DDL) {
    std::istringstream is("CREATE STREAM members (name STRING, age INT, income INT)");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::CREATE, lexer.next_token());
    EXPECT_EQ(Lexer::STREAM, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("MEMBERS", lexer.get_latest_name());

    EXPECT_EQ(Lexer::LPAREN, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("NAME", lexer.get_latest_name());
    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("STRING", lexer.get_latest_name());

    EXPECT_EQ(Lexer::COMMA, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("AGE", lexer.get_latest_name());
    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("INT", lexer.get_latest_name());

    EXPECT_EQ(Lexer::COMMA, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("INCOME", lexer.get_latest_name());
    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("INT", lexer.get_latest_name());

    EXPECT_EQ(Lexer::RPAREN, lexer.next_token());

    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}

TEST (testLexer, IN) {
    std::istringstream is("WHERE X IN Y");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::WHERE, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("X", lexer.get_latest_name());

    EXPECT_EQ(Lexer::IN, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("Y", lexer.get_latest_name());

    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}

TEST (TestDDL, TABLE) {
    std::istringstream is("CREATE TABLE members (name STRING, age INT, income INT)");
    currentia::Lexer lexer(&is);

    EXPECT_EQ(Lexer::CREATE, lexer.next_token());
    EXPECT_EQ(Lexer::TABLE, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("MEMBERS", lexer.get_latest_name());

    EXPECT_EQ(Lexer::LPAREN, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("NAME", lexer.get_latest_name());
    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("STRING", lexer.get_latest_name());

    EXPECT_EQ(Lexer::COMMA, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("AGE", lexer.get_latest_name());
    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("INT", lexer.get_latest_name());

    EXPECT_EQ(Lexer::COMMA, lexer.next_token());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("INCOME", lexer.get_latest_name());
    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("INT", lexer.get_latest_name());

    EXPECT_EQ(Lexer::RPAREN, lexer.next_token());

    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}
