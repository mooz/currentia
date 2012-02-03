#include <gtest/gtest.h>

#include "currentia/query/parser.h"

using namespace currentia;

template<typename T>
std::tr1::shared_ptr<T>
parse_string(const std::string& source) {
    std::istringstream is(source);
    Lexer::ptr_t lexer(new Lexer(&is));
    Parser parser(lexer);

    return std::tr1::dynamic_pointer_cast<T>(parser.parse_conditions());
}

TEST (testLexer, comparatorConjunctive) {
    Condition::ptr_t create_node_ptr = parse_string<Condition>(
        "x > 20 AND y >= 20 OR NOT z < 12.34"
    );

    EXPECT_EQ(
        "X > INT(20) && Y >= INT(20) || !(Z < FLOAT(12.34))",
        create_node_ptr->toString()
    );
}
