// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_SINGLE_INPUT_H_
#define CURRENTIA_OPERATOR_SINGLE_INPUT_H_

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

#include "currentia/core/operator/operator.h"

namespace currentia {
    class SingleInputOperator: public Operator {
    protected:
        Operator::ptr_t parent_operator_ptr_;
        Stream::ptr_t input_stream_;

        SingleInputOperator(Operator::ptr_t parent_operator_ptr):
            parent_operator_ptr_(parent_operator_ptr),
            input_stream_(parent_operator_ptr_->get_output_stream()) {
        }

        virtual ~SingleInputOperator() = 0;
    };

    SingleInputOperator::~SingleInputOperator() {}
}

#endif  /* ! CURRENTIA_OPERATOR_SINGLE_INPUT_H_ */
