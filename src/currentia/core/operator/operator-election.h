// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_ELECTION_H_
#define CURRENTIA_OPERATOR_ELECTION_H_

#include "currentia/core/tuple.h"
#include "currentia/core/operator/operator.h"

namespace currentia {
    // Election (Aggregation operator)
    class OperatorElection: public Operator {
        Operator::ptr_t parent_operator_ptr_;
        int window_width_;
        int current_window_position_;

        bool is_outputtable_() {
            return ++current_window_position_ % window_width_ == 0;
        }

    public:
        OperatorElection(Operator::ptr_t parent_operator_ptr, int window_width):
            // save arguments as private member
            parent_operator_ptr_(parent_operator_ptr),
            window_width_(window_width),
            current_window_position_(0) {
        }

        Schema::ptr_t get_output_schema_ptr() {
            return parent_operator_ptr_->get_output_schema_ptr();
        }

        Tuple::ptr_t next_implementation() {
            while (true) {
                Tuple::ptr_t target_tuple_ptr = parent_operator_ptr_->next();
                if (target_tuple_ptr->is_system_message())
                    return target_tuple_ptr;
                if (is_outputtable_())
                    return target_tuple_ptr;
            }

            // never comes here
            return Tuple::ptr_t();
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_ELECTION_H_ */
