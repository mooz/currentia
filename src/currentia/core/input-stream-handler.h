// -*- c++ -*-

#ifndef CURRENTIA_INPUT_STREAM_HANDLER_H_
#define CURRENTIA_INPUT_STREAM_HANDLER_H_

#include "currentia/core/stream-handler.h"

namespace currentia {
    class InputStreamHandler: public StreamHandler {
    public:
        InputStreamHandler(const Stream::ptr_t& stream_ptr,
                           const Schema::ptr_t& schema_ptr):
            StreamHandler(stream_ptr, schema_ptr) {
        }

        void enqueue(const Tuple::ptr_t& tuple) {
            stream_ptr_->enqueue(tuple);
        }

        void enqueue(const Tuple::data_t& data) {
            return this->enqueue(create_tuple(data));
        }
    };
}

#endif  /* ! CURRENTIA_INPUT_STREAM_HANDLER_H_ */
