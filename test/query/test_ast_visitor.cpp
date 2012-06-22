#include <gtest/gtest.h>

// core
#include "currentia/core/pointer.h"

// visitor
#include "currentia/query/ast/visitor.h"

// nodes
#include "currentia/query/ast/create.h"

using namespace currentia;

TEST (testAstVisitor, visitor) {
    std::list<Attribute> attributes;
    Create::ptr_t create_node(
        new Create(
            "Test Node",
            Create::ptr_t(new Create("Left Child")),
            Create::ptr_t(new Create("Right Child"))
        )
    );

    Visitor visitor;
    visitor.visit(*create_node);
}
