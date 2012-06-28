#include <gtest/gtest.h>

#include "currentia/core/stream.h"

using namespace currentia;

class TestStream : public ::testing::Test {
protected:
    Schema::ptr_t man_schema;
    Stream::ptr_t stream;

    TestStream():
        man_schema(create_man_schema()),
        stream(Stream::from_schema(man_schema)) {
    }

    virtual ~TestStream() {
    }

    Schema::ptr_t create_man_schema() {
        Schema::ptr_t schema_ptr(new Schema);
        schema_ptr->add_attribute("NAME", Object::STRING);
        schema_ptr->add_attribute("AGE", Object::INT);
        schema_ptr->freeze();

        return schema_ptr;
    }

    Tuple::ptr_t create_man_tuple(const std::string& name, int age) {
        return Tuple::create_easy(man_schema, name, age);
    }
};

TEST_F (TestStream, enqueue_dequeue) {
    Tuple::ptr_t tuple_a = create_man_tuple("A", 1);
    Tuple::ptr_t tuple_b = create_man_tuple("B", 2);
    Tuple::ptr_t tuple_c = create_man_tuple("C", 3);

    stream->enqueue(tuple_a);
    stream->enqueue(tuple_b);
    stream->enqueue(tuple_c);

    EXPECT_EQ(tuple_a, stream->dequeue());
    EXPECT_EQ(tuple_b, stream->dequeue());
    EXPECT_EQ(tuple_c, stream->dequeue());
}

TEST_F (TestStream, clear) {
    Tuple::ptr_t tuple_a = create_man_tuple("A", 1);
    Tuple::ptr_t tuple_b = create_man_tuple("B", 2);
    Tuple::ptr_t tuple_c = create_man_tuple("C", 3);

    stream->enqueue(tuple_a);
    stream->enqueue(tuple_b);

    stream->clear();

    stream->enqueue(tuple_c);

    EXPECT_EQ(tuple_c, stream->dequeue());
}

TEST_F (TestStream, insert) {
    Tuple::ptr_t tuple_a = create_man_tuple("A", 1);
    Tuple::ptr_t tuple_b = create_man_tuple("B", 2);
    Tuple::ptr_t tuple_c = create_man_tuple("C", 3);

    stream->enqueue(tuple_a);
    stream->enqueue(tuple_b);
    stream->enqueue(tuple_c);

    Tuple::ptr_t tuple_d = create_man_tuple("D", 4);
    Tuple::ptr_t tuple_e = create_man_tuple("E", 5);

    auto another_stream = Stream::from_schema(man_schema);
    another_stream->enqueue(tuple_d);
    another_stream->enqueue(tuple_e);

    stream->insert_head(another_stream);

    EXPECT_EQ(tuple_d->toString(), stream->dequeue()->toString());
    EXPECT_EQ(tuple_e->toString(), stream->dequeue()->toString());
    EXPECT_EQ(tuple_a->toString(), stream->dequeue()->toString());
    EXPECT_EQ(tuple_b->toString(), stream->dequeue()->toString());
    EXPECT_EQ(tuple_c->toString(), stream->dequeue()->toString());
}
