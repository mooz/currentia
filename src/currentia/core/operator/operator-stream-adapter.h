// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_STREAM_ADAPTER_H_
#define CURRENTIA_OPERATOR_STREAM_ADAPTER_H_

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"

namespace currentia {
    class OperatorStreamAdapter: public Operator {
        Stream::ptr_t input_stream_ptr_;
        Schema::ptr_t output_schema_ptr_;

    public:
        OperatorStreamAdapter(Stream::ptr_t input_stream_ptr):
            input_stream_ptr_(input_stream_ptr) {
            output_schema_ptr_ = input_stream_ptr->get_schema_ptr();
        }

        inline
        Schema::ptr_t get_output_schema_ptr() {
            return output_schema_ptr_;
        }

        inline
        Tuple::ptr_t next_implementation() {
            return input_stream_ptr_->dequeue();
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_STREAM_ADAPTER_H_ */
