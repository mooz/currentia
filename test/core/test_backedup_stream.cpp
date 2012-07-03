#include <gtest/gtest.h>

#include "currentia/core/stream.h"
#include "currentia/util/print.h"
#include "currentia/util/log.h"

using namespace currentia;

class TestStream : public ::testing::Test {
protected:
    Schema::ptr_t man_schema;
    BackupStream::ptr_t stream;

    TestStream():
        man_schema(create_man_schema()),
        stream(BackupStream::from_schema(man_schema)) {
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

    EXPECT_EQ(tuple_a, stream->non_blocking_dequeue());
    EXPECT_EQ(tuple_b, stream->non_blocking_dequeue());
    EXPECT_EQ(tuple_c, stream->non_blocking_dequeue());

    stream->recover_from_backup();

    EXPECT_EQ(tuple_a, stream->non_blocking_dequeue());
    EXPECT_EQ(tuple_b, stream->non_blocking_dequeue());
    EXPECT_EQ(tuple_c, stream->non_blocking_dequeue());
}

TEST_F (TestStream, eviction) {
    Tuple::ptr_t tuple_a = create_man_tuple("A", 1);
    Tuple::ptr_t tuple_b = create_man_tuple("B", 2);
    Tuple::ptr_t tuple_c = create_man_tuple("C", 3);
    Tuple::ptr_t tuple_d = create_man_tuple("D", 4);
    Tuple::ptr_t tuple_e = create_man_tuple("E", 5);

    stream->enqueue(tuple_a);
    stream->enqueue(tuple_b);
    stream->enqueue(tuple_c);
    stream->enqueue(tuple_d);
    stream->enqueue(tuple_e);

    stream->evict_backup_tuples_older_than(tuple_c->get_arrived_time());
    stream->clear();
    stream->recover_from_backup();

    EXPECT_EQ(tuple_c, stream->non_blocking_dequeue());
    EXPECT_EQ(tuple_d, stream->non_blocking_dequeue());
    EXPECT_EQ(tuple_e, stream->non_blocking_dequeue());
}
