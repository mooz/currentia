// -*- c++ -*-

#ifndef CURRENTIA_server_H__
#define CURRENTIA_server_H__

#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"
#include "currentia/core/schema.h"

#include <unistd.h>

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
        data.push_back(Object("John Smith"));
        data.push_back(Object(23));
        data.push_back(Object(10000));

        return Tuple::create(schema_ptr, data);
    }

    // Server class
    class Server {
        Schema::ptr_t schema_ptr_;
        Stream::ptr_t stream_ptr_;

    public:
        typedef std::tr1::shared_ptr<Server> ptr_t;

        Server(Schema::ptr_t schema):
            schema_ptr_(schema),
            stream_ptr_(new Stream(schema)) {
        }

        void listen() {
            while (true) {
                stream_ptr_->enqueue(create_tuple(schema_ptr_));
                // std::cout << "Listen!" << std::endl;
                sleep(1);
            }
        }

        void process() {
            while (Tuple::ptr_t tuple_ptr = stream_ptr_->dequeue()) {
                std::cout << "Got tuple!" << std::endl;
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

#endif  /* ! CURRENTIA_server_H__ */
