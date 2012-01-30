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
