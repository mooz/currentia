// -*- c++ -*-

#ifndef CURRENTIA_OUTPUT_STREAM_HANDLER_H_
#define CURRENTIA_OUTPUT_STREAM_HANDLER_H_

#include "currentia/core/stream-handler.h"

namespace currentia {
    class OutputStreamHandler: public StreamHandler {
    public:
        OutputStreamHandler(const Stream::ptr_t& stream_ptr,
                            const Schema::ptr_t& schema_ptr):
            StreamHandler(stream_ptr, schema_ptr) {
        }

        Tuple::ptr_t dequeue() {
            return stream_ptr_->dequeue();
        }
    };
}

#endif  /* ! CURRENTIA_OUTPUT_STREAM_HANDLER_H_ */
