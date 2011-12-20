// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_SELECTION_H__
#define CURRENTIA_OPERATOR_SELECTION_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/condition.h"

namespace currentia {
    class OperatorSelection: public Operator {
        Condition::ptr_t condition_ptr_;
        Operator::ptr_t parent_operator_ptr_;

    public:
        OperatorSelection(Operator::ptr_t parent_operator_ptr,
                          Condition::ptr_t condition_ptr):
            condition_ptr_(condition_ptr) {
            parent_operator_ptr_ = parent_operator_ptr;
            output_schema_ptr_ = parent_operator_ptr_->output_schema_ptr_;
        }

        Tuple::ptr_t next() {
            while (Tuple::ptr_t target_tuple_ptr = parent_operator_ptr_->next()) {
                if (condition_ptr_->check(target_tuple_ptr))
                    return target_tuple_ptr;
                // otherwise, tuple will be *deleted* by the shared_ptr
            }
            return Tuple::ptr_t(); // NULL
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_SELECTION_H__ */
