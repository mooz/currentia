// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_SELECTION_H_
#define CURRENTIA_OPERATOR_SELECTION_H_

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/condition.h"

#include "currentia/trait/pointable.h"

namespace currentia {
    class OperatorSelection: public Operator,
                             public Pointable<OperatorSelection> {
        Condition::ptr_t condition_ptr_;
        Operator::ptr_t parent_operator_ptr_;
        Schema::ptr_t output_schema_ptr_;

        int input_tuple_count_;
        int selected_tuple_count_;

    public:
        typedef Pointable<OperatorSelection>::ptr_t ptr_t;

        OperatorSelection(Operator::ptr_t parent_operator_ptr,
                          Condition::ptr_t condition_ptr):
            condition_ptr_(condition_ptr),
            input_tuple_count_(0),
            selected_tuple_count_(0) {
            parent_operator_ptr_ = parent_operator_ptr;
            output_schema_ptr_ = parent_operator_ptr_->get_output_schema_ptr();
        }

        Schema::ptr_t get_output_schema_ptr() {
            return output_schema_ptr_;
        }

        Tuple::ptr_t next_implementation() {
            while (Tuple::ptr_t target_tuple_ptr = parent_operator_ptr_->next()) {
                if (target_tuple_ptr->is_system_message())
                    return target_tuple_ptr;

                input_tuple_count_++;

                if (condition_ptr_->check(target_tuple_ptr)) {
                    selected_tuple_count_++;
                    return target_tuple_ptr;
                }
                // otherwise, tuple will be *deleted* by the shared_ptr
            }
            return Tuple::ptr_t(); // NULL
        }

        double get_selectivity() {
            if (input_tuple_count_ == 0)
                return 0.0;
            return static_cast<double>(selected_tuple_count_) / static_cast<double>(input_tuple_count_);
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_SELECTION_H_ */
