// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_H__
#define CURRENTIA_OPERATOR_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"

namespace currentia {
    class Operator {
    public:
        // input / output
        Stream::ptr_t input_stream_ptr_;
        Stream::ptr_t output_stream_ptr_;

        // open
        Operator(Stream::ptr_t input_stream_ptr):
            input_stream_ptr_(input_stream_ptr) {
        };

        // get next tuple from input stream and process
        virtual Tuple::ptr_t next() = 0; // blocking operator
    };
}

#endif  /* ! CURRENTIA_OPERATOR_H__ */
