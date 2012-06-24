#include <gtest/gtest.h>

#include "currentia/core/object.h"

using namespace currentia;

TEST (TestObject, equality) {
    Object number1(10);
    Object number2(10);

    EXPECT_TRUE(number1.compare(number2, Comparator::EQUAL));
}
