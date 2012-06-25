// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_SELECTION_H_
#define CURRENTIA_OPERATOR_SELECTION_H_

#include "currentia/core/object.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class OperatorSelection: public SingleInputOperator,
                             public Pointable<OperatorSelection> {
        Condition::ptr_t condition_ptr_;

        int input_tuple_count_;
        int selected_tuple_count_;

    public:
        typedef Pointable<OperatorSelection>::ptr_t ptr_t;

        OperatorSelection(Operator::ptr_t parent_operator_ptr,
                          Condition::ptr_t condition_ptr):
            SingleInputOperator(parent_operator_ptr),
            // Initialize members
            condition_ptr_(condition_ptr),
            input_tuple_count_(0),
            selected_tuple_count_(0) {
            // Arrange an output stream
            set_output_stream(Stream::from_schema(parent_operator_ptr->get_output_schema_ptr()));
        }

        Tuple::ptr_t process_single_input(Tuple::ptr_t input_tuple) {
            input_tuple_count_++;
            if (condition_ptr_->check(input_tuple)) {
                selected_tuple_count_++;
                output_tuple(input_tuple);
            }
            return Tuple::ptr_t();
        }

        double get_selectivity() {
            if (input_tuple_count_ == 0)
                return 0.0;
            return static_cast<double>(selected_tuple_count_) / static_cast<double>(input_tuple_count_);
        }

    public:
        virtual std::string toString() {
            return "Selection";
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_SELECTION_H_ */
