// -*- c++ -*-

#ifndef CURRENTIA_SERVER_H__
#define CURRENTIA_SERVER_H__

#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"
#include "currentia/core/schema.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/operator-selection.h"
#include "currentia/core/operator/operator-projection.h"
#include "currentia/core/operator/operator-join.h"
#include "currentia/core/operator/operator-stream-adapter.h"

#include <unistd.h>
#include <stdlib.h>
#include <sstream>

namespace currentia {
    // Helper functions
    Schema::ptr_t create_schema()
    {
        Schema::ptr_t schema_ptr(new Schema());
        schema_ptr->add_attribute("Name", Object::STRING);
        schema_ptr->add_attribute("Age", Object::INT);
        schema_ptr->add_attribute("Income", Object::INT);
        schema_ptr->freeze();

        return schema_ptr;
    }

    Tuple::ptr_t create_tuple(Schema::ptr_t schema_ptr)
    {
        // inefficient! (copy constructor of data_t runs many times)
        Tuple::data_t data;

        std::stringstream name_ss;
        static int number = 0;
        name_ss << "John Smith-" << (++number);
        std::string name = name_ss.str();

        data.push_back(Object(name));
        data.push_back(Object(rand() % 20 + 5));
        data.push_back(Object(rand() % 10000));

        return Tuple::create(schema_ptr, data);
    }

    // Server class
    class Server {
        Schema::ptr_t schema_ptr_;
        Stream::ptr_t stream_ptr_;
        Stream::ptr_t stream2_ptr_;

    public:
        typedef std::tr1::shared_ptr<Server> ptr_t;

        Server(Schema::ptr_t schema):
            schema_ptr_(schema),
            stream_ptr_(new Stream(schema)),
            stream2_ptr_(new Stream(schema)) {
        }

        void listen() {
            while (true) {
                stream_ptr_->enqueue(create_tuple(schema_ptr_));
                stream2_ptr_->enqueue(create_tuple(schema_ptr_));
                // std::cout << "Listen!" << std::endl;
                // sleep(1);
            }
        }

        void process() {
            OperatorStreamAdapter adapter(stream_ptr_);
            OperatorStreamAdapter adapter2(stream2_ptr_);
            Operator* output_ptr = NULL;
#if 0
            Condition::ptr_t condition1(
                new ConditionConstantComparator(std::string("Age"),
                                                Comparator::LESS_THAN,
                                                Object(10))
                );

            Condition::ptr_t condition2(
                new ConditionConstantComparator(std::string("Income"),
                                                Comparator::GREATER_THAN,
                                                Object(2000))
                );

            Condition::ptr_t condition1_and_2(
                new ConditionConjunctive(condition1, condition2, ConditionConjunctive::AND)
                );

            OperatorSelection selection(Operator::ptr_t(&adapter), condition1_and_2);

            OperatorProjection::target_attribute_names_t attribute_names;
            attribute_names.push_back(std::string("Age"));
            attribute_names.push_back(std::string("Name"));

            OperatorProjection projection(Operator::ptr_t(&selection), attribute_names);
            output_ptr = &projection;
#else
            Condition::ptr_t condition(
                new ConditionAttributeComparator(
                    std::string("Income"),
                    Comparator::EQUAL,
                    std::string("Income")
                    )
                );

            OperatorJoin join(Operator::ptr_t(&adapter),
                              Window(5, 1),
                              Operator::ptr_t(&adapter2),
                              Window(10, 2),
                              condition);
            output_ptr = &join;
#endif

            while (Tuple::ptr_t tuple_ptr = output_ptr->next()) {
                std::cout << "Got => " << tuple_ptr->toString()
                          << " at " << tuple_ptr->get_arrived_time() << std::endl;
            }
        }
    };

    void listen_thread_body(Server* server)
    {
        server->listen();
    }

    void process_thread_body( Server* server)
    {
        server->process();
    }
}

#endif  /* ! CURRENTIA_SERVER_H__ */
