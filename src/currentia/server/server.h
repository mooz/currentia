// -*- c++ -*-

#ifndef CURRENTIA_SERVER_H_
#define CURRENTIA_SERVER_H_

#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"
#include "currentia/core/schema.h"

#include "currentia/core/input-stream-handler.h"
#include "currentia/core/output-stream-handler.h"

#include "currentia/query/parser.h"

#include "currentia/trait/non-copyable.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/operator-selection.h"
#include "currentia/core/operator/operator-projection.h"
#include "currentia/core/operator/operator-join.h"
#include "currentia/core/operator/operator-stream-adapter.h"

#include <unistd.h>
#include <cstdlib>
#include <sstream>

namespace currentia {
    // Server class
    class Server: private NonCopyable<Server>,
                  public Pointable<Server> {
        std::map<std::string, Schema::ptr_t> schema_table_;
        std::map<std::string, Stream::ptr_t> stream_table_;
        std::map<std::string, Operator::ptr_t> query_table_;

    public:

        Server() {
        }

        OperatorStreamAdapter::ptr_t
        get_stream_adapter_by_name(const std::string& name) {
            // TODO: routing is needed. Do not create stream adapter
            return OperatorStreamAdapter::ptr_t(
                new OperatorStreamAdapter(stream_table_[name])
            );
        }

        bool
        has_stream_with_name(const std::string& name) {
            return stream_table_.find(name) != stream_table_.end();
        }

        // TODO: returns StreamHander (which allows user to enqueue tuples)
        InputStreamHandler::ptr_t
        define_stream(const std::string& name,
                      const std::list<Attribute>& attributes) {
            if (has_stream_with_name(name))
                return InputStreamHandler::ptr_t(); // NULL

            // create schema
            Schema::ptr_t schema_ptr(new Schema());

            std::list<Attribute>::const_iterator attribute_iter = attributes.begin();
            for (; attribute_iter != attributes.end(); ++attribute_iter) {
                schema_ptr->add_attribute(attribute_iter->name,
                                          attribute_iter->type);
            }

            schema_ptr->freeze();
            schema_table_[name] = schema_ptr;

            // create stream
            Stream::ptr_t stream_ptr(new Stream(schema_ptr));
            stream_table_[name] = stream_ptr;

            return InputStreamHandler::ptr_t(new InputStreamHandler(stream_ptr, schema_ptr));
        }

        Operator::ptr_t
        register_query_lagacy(const std::string& query_name,
                              Operator::ptr_t operator_ptr) {
            query_table_[query_name] = operator_ptr;
            // routing
            return operator_ptr;
        }

        // std::list<std::string>::iterator get_defined_stream_names() const {
        //     return stream_table_.begin();
        // }

        void
        listen() {
        }

        void
        process() {
#if 0
            OperatorStreamAdapter adapter(stream_ptr_);
            OperatorStreamAdapter adapter2(stream2_ptr_);
            Operator* output_ptr = NULL;

            std::stringstream is("AGE < 10");
            Condition::ptr_t condition1_and_2 = Parser::parse_conditions_from_stream(is);

            std::cout << condition1_and_2->toString() << std::endl;
            condition1_and_2->to_cnf();
            std::cout << condition1_and_2->toString() << std::endl;

            exit(0);

            OperatorSelection selection(Operator::ptr_t(&adapter), condition1_and_2);

            OperatorProjection::target_attribute_names_t attribute_names;
            attribute_names.push_back(std::string("Age"));
            attribute_names.push_back(std::string("Name"));

            OperatorProjection projection(Operator::ptr_t(&selection), attribute_names);
            output_ptr = &projection;

            std::stringstream is("INCOME = INCOME");
            Condition::ptr_t condition = Parser::parse_conditions_from_stream(is);

            OperatorJoin join(Operator::ptr_t(&adapter),
                              Window(5, 1),
                              Operator::ptr_t(&adapter2),
                              Window(10, 2),
                              condition);
            output_ptr = &join;


            while (Tuple::ptr_t tuple_ptr = output_ptr->next()) {
                std::cout << "Got => " << tuple_ptr->toString()
                          << " at " << tuple_ptr->get_arrived_time() << std::endl;
            }

#endif
        }
    };

    // for pthread

    void listen_thread_body(Server* server) {
        server->listen();
    }

    void process_thread_body(Server* server) {
        server->process();
    }
}

#endif  /* ! CURRENTIA_SERVER_H_  */
