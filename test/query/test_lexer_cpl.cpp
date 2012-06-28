#include <gtest/gtest.h>

#include "currentia/query/cpl-lexer.h"

using namespace currentia;

TEST (TestDDL, TABLE) {
    std::istringstream is(
        " stream purchases(goods_id: int, user_id: int)"
        " relation goods(id: int, price: int)"
        " stream result {"
        "   join purchases with goods where purchases.id = goods.goods_id"
        "   selection price < 5000"
        "   mean goods.price width 5 slide 5"
        " }"
    );
    currentia::CPLLexer lexer(&is);

#define EXPECT_NEXT_TOKEN(TOKEN) EXPECT_EQ(TOKEN, lexer.get_next_token())

    // -------------------------------
    EXPECT_NEXT_TOKEN(TOKEN_STREAM);
    // purchases
    EXPECT_NEXT_TOKEN(TOKEN_NAME);

    // goods_id
    EXPECT_NEXT_TOKEN(TOKEN_LPAREN); // (
    EXPECT_NEXT_TOKEN(TOKEN_NAME);
    EXPECT_EQ("goods_id", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_COLON); // :
    EXPECT_NEXT_TOKEN(TOKEN_TYPE_INT);

    EXPECT_NEXT_TOKEN(TOKEN_COMMA); // ,

    // user_id
    EXPECT_NEXT_TOKEN(TOKEN_NAME);
    EXPECT_EQ("user_id", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_COLON); // :
    EXPECT_NEXT_TOKEN(TOKEN_TYPE_INT);
    EXPECT_NEXT_TOKEN(TOKEN_RPAREN); // )

    // -------------------------------
    EXPECT_NEXT_TOKEN(TOKEN_RELATION);
    // goods
    EXPECT_NEXT_TOKEN(TOKEN_NAME);
    EXPECT_EQ("goods", lexer.get_token_text());

    // id
    EXPECT_NEXT_TOKEN(TOKEN_LPAREN); // (
    EXPECT_NEXT_TOKEN(TOKEN_NAME);
    EXPECT_EQ("id", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_COLON); // :
    EXPECT_NEXT_TOKEN(TOKEN_TYPE_INT);

    EXPECT_NEXT_TOKEN(TOKEN_COMMA); // ,

    // price
    EXPECT_NEXT_TOKEN(TOKEN_NAME);
    EXPECT_EQ("price", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_COLON); // :
    EXPECT_NEXT_TOKEN(TOKEN_TYPE_INT);
    EXPECT_NEXT_TOKEN(TOKEN_RPAREN); // )

    // -------------------------------
    EXPECT_NEXT_TOKEN(TOKEN_STREAM);
    // result
    EXPECT_NEXT_TOKEN(TOKEN_NAME);
    EXPECT_EQ("result", lexer.get_token_text());

    EXPECT_NEXT_TOKEN(TOKEN_LBRACE); // {
    // join
    EXPECT_NEXT_TOKEN(TOKEN_COMBINE);
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // purchases
    EXPECT_EQ("purchases", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_WITH);
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // goods
    EXPECT_EQ("goods", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_WHERE);
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // purchases
    EXPECT_EQ("purchases", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_DOT); // .
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // id
    EXPECT_EQ("id", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_EQUAL); // =
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // goods
    EXPECT_EQ("goods", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_DOT); // .
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // goods_id
    EXPECT_EQ("goods_id", lexer.get_token_text());
    // selection
    EXPECT_NEXT_TOKEN(TOKEN_SELECT);
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // price
    EXPECT_EQ("price", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_LESS_THAN); // <
    EXPECT_NEXT_TOKEN(TOKEN_INTEGER); // 5000
    // mean
    EXPECT_NEXT_TOKEN(TOKEN_MEAN);
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // goods
    EXPECT_EQ("goods", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_DOT); // .
    EXPECT_NEXT_TOKEN(TOKEN_NAME); // price
    EXPECT_EQ("price", lexer.get_token_text());
    EXPECT_NEXT_TOKEN(TOKEN_WIDTH);
    EXPECT_NEXT_TOKEN(TOKEN_INTEGER); // 5
    EXPECT_NEXT_TOKEN(TOKEN_SLIDE);
    EXPECT_NEXT_TOKEN(TOKEN_INTEGER); // 5
    // end
    EXPECT_NEXT_TOKEN(TOKEN_RBRACE); // }

    EXPECT_EQ(TOKEN_EOS, lexer.get_next_token());
}
