// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_SINGLE_INPUT_H_
#define CURRENTIA_OPERATOR_SINGLE_INPUT_H_

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/visitable-operator.h"
#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class SingleInputOperator: public Operator,
                               public VisitableOperator<SingleInputOperator> {
    protected:
        Operator::ptr_t parent_operator_ptr_;
        Stream::ptr_t input_stream_;

        SingleInputOperator(Operator::ptr_t parent_operator_ptr):
            Operator(),
            parent_operator_ptr_(parent_operator_ptr),
            input_stream_(parent_operator_ptr_->get_output_stream()) {
        }

        virtual ~SingleInputOperator() = 0;

        void next_implementation() {
            Tuple::ptr_t input_tuple = input_stream_->non_blocking_dequeue();
            if (!input_tuple)
                return;

#ifdef CURRENTIA_CHECK_STATISTICS
            evaluation_count_++;
#endif

            if (input_tuple->is_system_message()) {
                output_tuple(input_tuple);
                return;
            }

            process_single_input(input_tuple);
        }

        virtual void process_single_input(Tuple::ptr_t input_tuple) = 0;

    public:
        const Operator::ptr_t get_parent_operator() const {
            return parent_operator_ptr_;
        }
    };

    SingleInputOperator::~SingleInputOperator() {}
}

#endif  /* ! CURRENTIA_OPERATOR_SINGLE_INPUT_H_ */
