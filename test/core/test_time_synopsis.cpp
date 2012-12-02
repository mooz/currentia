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

    TestSynopsis():
        window(3500, 500, Window::TIME_BASE),
        synopsis(create_synopsis_from_window(window)),
        schema(new Schema()) {
        schema->add_attribute("foo", Object::INT);
        schema->freeze();
    }

    virtual ~TestSynopsis() {
    }
};

TEST_F (TestSynopsis, has_consistency) {
    std::clog.rdbuf(new system::Log("currentia"));

    synopsis->enqueue(Tuple::create_easy(schema, 1)); // 0
    sleep(1);
    synopsis->enqueue(Tuple::create_easy(schema, 1)); // 1 sec elapsed
    sleep(1);
    synopsis->enqueue(Tuple::create_easy(schema, 1)); // 2 sec elapsed
    sleep(1);
    synopsis->enqueue(Tuple::create_easy(schema, 1)); // 3 sec elapsed

    std::clog << synopsis->toString() << std::endl;

    sleep(1);
    synopsis->enqueue(Tuple::create_easy(schema, 1)); // 4 sec elapsed

    std::clog << synopsis->toString() << std::endl;

    sleep(1);
    synopsis->enqueue(Tuple::create_easy(schema, 1)); // 5 sec elapsed

    sleep(5);
    synopsis->enqueue(Tuple::create_easy(schema, 1)); // 10 sec elapsed
    std::clog << synopsis->toString() << std::endl;

    EXPECT_TRUE(true);
}
