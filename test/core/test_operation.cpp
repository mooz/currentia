#include <gtest/gtest.h>

#include "currentia/core/operation/operations.h"

using namespace currentia;
using namespace currentia::operations;

TEST (TestOperation, add) {
    EXPECT_TRUE(operation_add(Object(7), Object(3))->compare(Object(10), Comparator::EQUAL));
    EXPECT_TRUE(operation_add(Object(7), Object(-3))->compare(Object(4), Comparator::EQUAL));

    EXPECT_TRUE(operation_add(Object(1.3), Object(4.5))->compare(Object(5.8), Comparator::EQUAL));

    EXPECT_TRUE(operation_add(Object("abc"), Object("def"))->compare(Object("abcdef"), Comparator::EQUAL));
}

TEST (TestOperation, subtract) {
    EXPECT_TRUE(operation_subtract(Object(7), Object(3))->compare(Object(4), Comparator::EQUAL));
    EXPECT_TRUE(operation_subtract(Object(7), Object(-3))->compare(Object(10), Comparator::EQUAL));
    EXPECT_TRUE(operation_subtract(Object(3), Object(7))->compare(Object(-4), Comparator::EQUAL));

    EXPECT_TRUE(operation_subtract(Object(1.3), Object(0.3))->compare(Object(1.0), Comparator::EQUAL));

    EXPECT_THROW(operation_subtract(Object("abc"), Object("def")), OPERATION_ERROR);
}

TEST (TestOperation, multiply) {
    EXPECT_TRUE(operation_multiply(Object(7), Object(3))->compare(Object(21), Comparator::EQUAL));
    EXPECT_TRUE(operation_multiply(Object(7), Object(-3))->compare(Object(-21), Comparator::EQUAL));

    EXPECT_TRUE(operation_multiply(Object(1.3), Object(2.0))->compare(Object(2.6), Comparator::EQUAL));

    EXPECT_THROW(operation_subtract(Object(1.3), Object(2)), OPERATION_ERROR);
    EXPECT_THROW(operation_subtract(Object("abc"), Object("def")), OPERATION_ERROR);
}
