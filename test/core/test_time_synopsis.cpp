#include <gtest/gtest.h>

#include "currentia/core/stream.h"
#include "currentia/core/schema.h"
#include "currentia/core/operator/synopsis.h"

#include "currentia/util/print.h"
#include "currentia/util/log.h"

#include <unistd.h>

using namespace currentia;

std::ostream& operator<<(std::ostream& out, const Synopsis::const_iterator& p) {
    return out << p;
}

void sleep_sec(double sec) {
    usleep(static_cast<useconds_t>(sec * 1000 * 1000));
}

class TestTimeSynopsis : public ::testing::Test {
protected:
    Window window;
    Synopsis::ptr_t synopsis;
    Schema::ptr_t schema;
    Relation::ptr_t dummy_relation;

    Tuple::ptr_t t0, t1, t2, t3, t4, t5, t6;

    TestTimeSynopsis():
        window(3500, 1700, Window::TIME_BASE),
        synopsis(create_synopsis_from_window(window)),
        schema(new Schema()),
        invoked_count(0) {
        schema->add_attribute("foo", Object::INT);
        schema->freeze();
        // set
        synopsis->set_on_accept(std::bind(&TestTimeSynopsis::on_operator_invocation, this));
    }

    virtual ~TestTimeSynopsis() {
    }

    int invoked_count;
    void on_operator_invocation() {
        switch (invoked_count++) {
        case 0: {
            // std::clog << "0 ~ 3.5" << std::endl;
            // util::print_iterable_string(synopsis->begin(), synopsis->end());
            // 0 ~ 3.5
            auto iter = synopsis->begin();
            ASSERT_EQ(t0, *iter++);
            ASSERT_EQ(t1, *iter++);
            ASSERT_EQ(t2, *iter++);
            ASSERT_EQ(t3, *iter++);
            ASSERT_TRUE(synopsis->end() == iter);
            break;
        }
        case 1: {
            // std::clog << "1.7 ~ 5.2" << std::endl;
            // util::print_iterable_string(synopsis->begin(), synopsis->end());
            // 1.7 ~ 5.2 (1.7 + 3.5)
            auto iter = synopsis->begin();
            ASSERT_EQ(t2, *iter++);
            ASSERT_EQ(t3, *iter++);
            ASSERT_EQ(t4, *iter++);
            ASSERT_EQ(t5, *iter++);
            ASSERT_TRUE(synopsis->end() == iter);
            break;
        }
        case 2: {
            // std::clog << "3.4 ~ 6.9" << std::endl;
            // util::print_iterable_string(synopsis->begin(), synopsis->end());
            // 3.4 (1.7 + 1.7) ~ 6.9 (3.4 + 3.5)
            auto iter = synopsis->begin();
            ASSERT_EQ(t4, *iter++);
            ASSERT_EQ(t5, *iter++);
            ASSERT_EQ(t6, *iter++);
            ASSERT_EQ(synopsis->end(), iter);
            break;
        }
        default:
            FAIL() << "on_call called more than the expected count";
        }
    }
};

TEST_F (TestTimeSynopsis, test_equal_interval) {
    std::clog.rdbuf(new system::Log("currentia"));

    synopsis->enqueue(t0 = Tuple::create_easy(schema, 1)); // 0
    sleep(1);
    synopsis->enqueue(t1 = Tuple::create_easy(schema, 1)); // 1 sec elapsed
    sleep(1);
    synopsis->enqueue(t2 = Tuple::create_easy(schema, 1)); // 2 sec elapsed
    sleep(1);
    synopsis->enqueue(t3 = Tuple::create_easy(schema, 1)); // 3 sec elapsed
    sleep(1);
    synopsis->enqueue(t4 = Tuple::create_easy(schema, 1)); // 4 sec elapsed
    sleep(1);
    synopsis->enqueue(t5 = Tuple::create_easy(schema, 1)); // 5 sec elapsed
    sleep(1);
    synopsis->enqueue(t6 = Tuple::create_easy(schema, 1)); // 6 sec elapsed
}
