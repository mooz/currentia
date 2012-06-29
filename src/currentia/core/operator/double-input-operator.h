// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_DOUBLE_INPUT_H_
#define CURRENTIA_OPERATOR_DOUBLE_INPUT_H_

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/visitable-operator.h"
#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class DoubleInputOperator: public Operator,
                               public VisitableOperator<DoubleInputOperator> {
    protected:
        Operator::ptr_t parent_left_operator_ptr_;
        Operator::ptr_t parent_right_operator_ptr_;
        Stream::ptr_t left_input_stream_;
        Stream::ptr_t right_input_stream_;

        DoubleInputOperator(const Operator::ptr_t& parent_left_operator_ptr,
                            const Operator::ptr_t& parent_right_operator_ptr):
            parent_left_operator_ptr_(parent_left_operator_ptr),
            parent_right_operator_ptr_(parent_right_operator_ptr),
            left_input_stream_(parent_left_operator_ptr_->get_output_stream()),
            right_input_stream_(parent_right_operator_ptr_->get_output_stream()) {
        }

        virtual ~DoubleInputOperator() = 0;

        bool is_concrete_input(const Tuple::ptr_t& input_tuple) {
            if (!input_tuple)
                return false;

            if (input_tuple->is_system_message()) {
                output_tuple(input_tuple);
                return false;
            }

            return true;
        }

        void next_implementation() {
            Tuple::ptr_t left_input_tuple = left_input_stream_->non_blocking_dequeue();
            if (is_concrete_input(left_input_tuple))
                process_left_input(left_input_tuple);

            Tuple::ptr_t right_input_tuple = right_input_stream_->non_blocking_dequeue();
            if (is_concrete_input(right_input_tuple))
                process_right_input(right_input_tuple);
        }

        virtual void process_left_input(Tuple::ptr_t input) = 0;
        virtual void process_right_input(Tuple::ptr_t input) = 0;

    public:
        const Operator::ptr_t get_parent_left_operator() const {
            return parent_left_operator_ptr_;
        }

        const Operator::ptr_t get_parent_right_operator() const {
            return parent_right_operator_ptr_;
        }
    };

    DoubleInputOperator::~DoubleInputOperator() {}
}

#endif  /* ! CURRENTIA_OPERATOR_DOUBLE_INPUT_H_ */
