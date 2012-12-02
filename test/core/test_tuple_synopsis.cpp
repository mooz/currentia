#include <gtest/gtest.h>

#include "currentia/core/stream.h"
#include "currentia/core/schema.h"
#include "currentia/core/operator/synopsis.h"

#include "currentia/util/log.h"

using namespace currentia;

class TestSynopsis : public ::testing::Test {
protected:
    Window window;
    Synopsis::ptr_t synopsis;
    Schema::ptr_t schema;
    Relation::ptr_t dummy_relation;

    Tuple::ptr_t tuple_a;
    Tuple::ptr_t tuple_b;
    Tuple::ptr_t tuple_c;

    TestSynopsis():
        window(3, 3),
        synopsis(create_synopsis_from_window(window)),
        schema(new Schema()) {
        schema->add_attribute("foo", Object::INT);
        schema->freeze();

        dummy_relation = Relation::ptr_t(new Relation(schema));

        tuple_a = Tuple::create_easy(schema, 1);
        tuple_b = Tuple::create_easy(schema, 2);
        tuple_c = Tuple::create_easy(schema, 3);
    }

    virtual ~TestSynopsis() {
    }
};

TEST_F (TestSynopsis, has_consistency) {
    std::clog.rdbuf(new system::Log("currentia"));

    tuple_a->set_referenced_version_number(dummy_relation, 2);
    tuple_b->set_referenced_version_number(dummy_relation, 2);
    tuple_c->set_referenced_version_number(dummy_relation, 2);

    synopsis->enqueue(tuple_a);
    synopsis->enqueue(tuple_b);
    synopsis->enqueue(tuple_c);

    std::clog << tuple_c->toString() << std::endl;

    EXPECT_TRUE(synopsis->has_reference_consistency());
}

TEST_F (TestSynopsis, not_has_consistency) {
    std::clog.rdbuf(new system::Log("currentia"));

    tuple_a->set_referenced_version_number(dummy_relation, 1);
    tuple_b->set_referenced_version_number(dummy_relation, 2);
    tuple_c->set_referenced_version_number(dummy_relation, 3);

    synopsis->enqueue(tuple_a);
    synopsis->enqueue(tuple_b);
    synopsis->enqueue(tuple_c);

    EXPECT_FALSE(synopsis->has_reference_consistency());
}

TEST_F (TestSynopsis, multiple_relation) {
    std::clog.rdbuf(new system::Log("currentia"));

    Relation::ptr_t another_relation = dummy_relation->copy();

    tuple_a->set_referenced_version_number(dummy_relation, 1);
    tuple_b->set_referenced_version_number(dummy_relation, 1);
    tuple_c->set_referenced_version_number(dummy_relation, 1);

    tuple_a->set_referenced_version_number(another_relation, 1);
    tuple_b->set_referenced_version_number(another_relation, 1);
    tuple_c->set_referenced_version_number(another_relation, 1);

    synopsis->enqueue(tuple_a);
    synopsis->enqueue(tuple_b);
    synopsis->enqueue(tuple_c);

    EXPECT_TRUE(synopsis->has_reference_consistency());
}

TEST_F (TestSynopsis, multiple_relation_inconsistent) {
    std::clog.rdbuf(new system::Log("currentia"));

    Relation::ptr_t another_relation = dummy_relation->copy();

    tuple_a->set_referenced_version_number(dummy_relation, 1);
    tuple_b->set_referenced_version_number(dummy_relation, 1);
    tuple_c->set_referenced_version_number(dummy_relation, 1);

    tuple_a->set_referenced_version_number(another_relation, 1);
    tuple_b->set_referenced_version_number(another_relation, 2);
    tuple_c->set_referenced_version_number(another_relation, 1);

    synopsis->enqueue(tuple_a);
    synopsis->enqueue(tuple_b);
    synopsis->enqueue(tuple_c);

    EXPECT_FALSE(synopsis->has_reference_consistency());
}

TEST_F (TestSynopsis, multiple_relation_difference_state) {
    std::clog.rdbuf(new system::Log("currentia"));

    Relation::ptr_t another_relation = dummy_relation->copy();
    Relation::ptr_t another_relation2 = dummy_relation->copy();

    tuple_a->set_referenced_version_number(dummy_relation, 1);
    tuple_b->set_referenced_version_number(dummy_relation, 1);
    tuple_c->set_referenced_version_number(dummy_relation, 1);

    tuple_a->set_referenced_version_number(another_relation, 3);
    tuple_b->set_referenced_version_number(another_relation, 3);
    tuple_c->set_referenced_version_number(another_relation, 3);

    synopsis->enqueue(tuple_a);
    synopsis->enqueue(tuple_b);
    synopsis->enqueue(tuple_c);

    EXPECT_TRUE(synopsis->has_reference_consistency());
}
