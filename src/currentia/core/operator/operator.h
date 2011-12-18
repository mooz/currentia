// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_H__
#define CURRENTIA_OPERATOR_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"

namespace currentia {
    class Operator {
    public:
        // input / output
        Stream::ptr_t input_stream_;
        Stream::ptr_t output_stream_;

        // open
        Operator(Stream::ptr_t input_stream):
            input_stream_(input_stream) {
        };

        // get next tuple from input stream and process
        virtual Tuple::ptr_t next() = 0; // blocking operator
    };
}

#endif  /* ! CURRENTIA_OPERATOR_H__ */
