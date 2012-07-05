// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_STREAM_ADAPTER_H_
#define CURRENTIA_OPERATOR_STREAM_ADAPTER_H_

#include "currentia/core/object.h"
#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/visitable-operator.h"
#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"

namespace currentia {
    class OperatorStreamAdapter: public Operator,
                                 public VisitableOperator<OperatorStreamAdapter> {
        Stream::ptr_t input_stream_ptr_;

    public:
        OperatorStreamAdapter(Stream::ptr_t input_stream_ptr):
            Operator(),
            input_stream_ptr_(input_stream_ptr) {
            set_output_stream(Stream::from_schema(input_stream_ptr->get_schema()));
        }

        void next_implementation() {
            output_tuple(input_stream_ptr_->dequeue());
        }

        Stream::ptr_t get_input_stream() {
            return input_stream_ptr_;
        }

    public:
        std::string toString() const {
            return get_name();
        }

        std::string get_name() const {
            return std::string("Stream-Adapter");
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_STREAM_ADAPTER_H_ */
