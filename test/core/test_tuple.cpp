#include <gtest/gtest.h>

#include "currentia/core/tuple.h"

using namespace currentia;

class TestTuple : public ::testing::Test {
    Schema::ptr_t man_schema_;

protected:
    Tuple::ptr_t man_tuple_alice;
    Tuple::ptr_t man_tuple_bob;

    TestTuple():
        man_schema_(create_man_schema()) {
        man_tuple_alice = create_man_tuple("ALICE", 7);
        man_tuple_bob = create_man_tuple("BOB", 22);
    }

    virtual ~TestTuple() {
    }

    Schema::ptr_t create_man_schema() {
        Schema::ptr_t schema_ptr(new Schema);
        schema_ptr->add_attribute("NAME", Object::STRING);
        schema_ptr->add_attribute("AGE", Object::INT);
        schema_ptr->freeze();

        return schema_ptr;
    }

    Tuple::ptr_t create_man_tuple(const std::string& name, int age) {
        Tuple::data_t man_data;
        man_data.push_back(Object(name));
        man_data.push_back(Object(age));

        return Tuple::create(man_schema_, man_data);
    }
};

TEST_F (TestTuple, copy) {
}
