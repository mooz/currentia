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
        schema_ptr->add_attribute("Name", TYPE_STRING);
        schema_ptr->add_attribute("Age", TYPE_INT);
        schema_ptr->add_attribute("Income", TYPE_INT);
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
                sleep(1);
            }
        }

        void process() {
            OperatorStreamAdapter adapter(stream_ptr_);
            OperatorStreamAdapter adapter2(stream2_ptr_);

#if 0
            OperatorSelection selection(Operator::ptr_t(&adapter),
                                        COMPARATOR_LESS_THAN,
                                        std::string("Age"),
                                        Object(10));

            OperatorProjection::target_attribute_names_t attribute_names;
            attribute_names.push_back(std::string("Age"));
            attribute_names.push_back(std::string("Name"));

            OperatorProjection projection(Operator::ptr_t(&selection), attribute_names);
#endif

            OperatorJoin join(Operator::ptr_t(&adapter),
                              Operator::ptr_t(&adapter2),
                              COMPARATOR_EQUAL,
                              std::string("Age"),
                              1,
                              1);

            while (Tuple::ptr_t tuple_ptr = join.next()) {
                std::cout << "Got => " << tuple_ptr->toString() << std::endl;
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
