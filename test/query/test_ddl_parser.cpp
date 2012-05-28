#include <gtest/gtest.h>

#include "currentia/query/parser.h"

using namespace currentia;

template<typename T>
std::tr1::shared_ptr<T>
parse_string(const std::string& source) {
    std::istringstream is(source);
    Lexer::ptr_t lexer(new Lexer(&is));
    Parser parser(lexer);

    Statement::ptr_t root(parser.parse_statement());
    return std::tr1::dynamic_pointer_cast<T>(root);
}

TEST (testLexer, DDL) {
    CreateNode::ptr_t create_node_ptr = parse_string<CreateNode>(
        "CREATE STREAM members (name STRING, age INT, income INT)"
    );

    EXPECT_NE(CreateNode::ptr_t(), create_node_ptr) << "No Syntax Error";

    std::list<Attribute> attributes = create_node_ptr->attributes;
    std::list<Attribute>::iterator attribute_iter = attributes.begin();

    EXPECT_EQ("NAME", attribute_iter->name);
    EXPECT_EQ(Object::STRING, attribute_iter->type);

    attribute_iter++;

    EXPECT_EQ("AGE", attribute_iter->name);
    EXPECT_EQ(Object::INT, attribute_iter->type);

    attribute_iter++;

    EXPECT_EQ("INCOME", attribute_iter->name);
    EXPECT_EQ(Object::INT, attribute_iter->type);

    attribute_iter++;

    EXPECT_TRUE(attributes.end() == attribute_iter);
}
