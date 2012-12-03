#include <gtest/gtest.h>

#include "currentia/core/operation/operations.h"

using namespace currentia;

TEST (TestOperation, add) {
    EXPECT_TRUE(Operation::add(Object(7), Object(3)).compare(Object(10), Comparator::EQUAL));
    EXPECT_TRUE(Operation::add(Object(7), Object(-3)).compare(Object(4), Comparator::EQUAL));
    EXPECT_TRUE(Operation::add(Object(1.3), Object(4.5)).compare(Object(5.8), Comparator::EQUAL));
    EXPECT_TRUE(Operation::add(Object("abc"), Object("def")).compare(Object("abcdef"), Comparator::EQUAL));
    // With conversions
    EXPECT_TRUE(Operation::add(Object(7.5), Object(3)).compare(Object(10.5), Comparator::EQUAL));
    EXPECT_TRUE(Operation::add(Object(3), Object(7.5)).compare(Object(10.5), Comparator::EQUAL));
    EXPECT_TRUE(Operation::add(Object("abc"), Object(90)).compare(Object("abc90"), Comparator::EQUAL));
    EXPECT_TRUE(Operation::add(Object("def"), Object(8.3)).compare(Object("def8.3"), Comparator::EQUAL));
}

TEST (TestOperation, subtract) {
    EXPECT_TRUE(Operation::subtract(Object(7), Object(3)).compare(Object(4), Comparator::EQUAL));
    EXPECT_TRUE(Operation::subtract(Object(7), Object(-3)).compare(Object(10), Comparator::EQUAL));
    EXPECT_TRUE(Operation::subtract(Object(3), Object(7)).compare(Object(-4), Comparator::EQUAL));
    EXPECT_TRUE(Operation::subtract(Object(1.3), Object(0.3)).compare(Object(1.0), Comparator::EQUAL));
    EXPECT_THROW(Operation::subtract(Object("abc"), Object("def")), Operation::OPERATION_ERROR);
    // With conversions
    EXPECT_TRUE(Operation::subtract(Object(7.5), Object(3)).compare(Object(4.5), Comparator::EQUAL));
    EXPECT_TRUE(Operation::subtract(Object(3), Object(7.5)).compare(Object(-4.5), Comparator::EQUAL));
    EXPECT_THROW(Operation::subtract(Object("abc"), Object(90)), Operation::OPERATION_ERROR);
    EXPECT_THROW(Operation::subtract(Object("def"), Object(8.3)), Operation::OPERATION_ERROR);
}

TEST (TestOperation, multiply) {
    EXPECT_TRUE(Operation::multiply(Object(7), Object(3)).compare(Object(21), Comparator::EQUAL));
    EXPECT_TRUE(Operation::multiply(Object(7), Object(-3)).compare(Object(-21), Comparator::EQUAL));
    EXPECT_TRUE(Operation::multiply(Object(1.3), Object(2.0)).compare(Object(2.6), Comparator::EQUAL));
    EXPECT_THROW(Operation::multiply(Object("abc"), Object("def")), Operation::OPERATION_ERROR);
    // With conversions
    EXPECT_TRUE(Operation::multiply(Object(7), Object(1.5)).compare(Object(10.5), Comparator::EQUAL));
    EXPECT_TRUE(Operation::multiply(Object(1.3), Object(2)).compare(Object(2.6), Comparator::EQUAL));
    EXPECT_THROW(Operation::multiply(Object("abc"), Object(3.0)), Operation::OPERATION_ERROR);
}

TEST (TestOperation, divide) {
    EXPECT_TRUE(Operation::divide(Object(8), Object(2)).compare(Object(4), Comparator::EQUAL));
    EXPECT_TRUE(Operation::divide(Object(10), Object(-5)).compare(Object(-2), Comparator::EQUAL));
    EXPECT_TRUE(Operation::divide(Object(8.8), Object(2.0)).compare(Object(4.4), Comparator::EQUAL));
    EXPECT_THROW(Operation::divide(Object("abc"), Object("def")), Operation::OPERATION_ERROR);
    // With conversions
    EXPECT_TRUE(Operation::divide(Object(6), Object(1.5)).compare(Object(4.0), Comparator::EQUAL));
    EXPECT_TRUE(Operation::divide(Object(1.3), Object(2)).compare(Object(0.65), Comparator::EQUAL));
    EXPECT_THROW(Operation::divide(Object("abc"), Object(3.0)), Operation::OPERATION_ERROR);
}
