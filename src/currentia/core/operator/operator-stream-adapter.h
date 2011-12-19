// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_STREAM_ADAPTER_H__
#define CURRENTIA_OPERATOR_STREAM_ADAPTER_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"

namespace currentia {
    class OperatorStreamAdapter: public Operator {
        Stream::ptr_t input_stream_ptr_;

    public:
        OperatorStreamAdapter(Stream::ptr_t input_stream_ptr):
            input_stream_ptr_(input_stream_ptr) {
            output_schema_ptr_ = input_stream_ptr->schema_ptr_;
            parent_operator_ptr_ = Operator::ptr_t(); // NULL
        }

        Tuple::ptr_t next() {
            return input_stream_ptr_->dequeue();
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_STREAM_ADAPTER_H__ */
