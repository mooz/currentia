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

TEST (testLexer, keywords) {
    std::istringstream is("SELECT FROM WHERE");
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
    EXPECT_EQ("foo", lexer.get_latest_name());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("bar", lexer.get_latest_name());

    EXPECT_EQ(Lexer::NAME, lexer.next_token());
    EXPECT_EQ("baz", lexer.get_latest_name());

    EXPECT_EQ(Lexer::EOS, lexer.next_token());
}
