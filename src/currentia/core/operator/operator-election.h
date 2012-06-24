// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_ELECTION_H_
#define CURRENTIA_OPERATOR_ELECTION_H_

#include "currentia/core/tuple.h"
#include "currentia/core/operator/single-input-operator.h"

namespace currentia {
    // Election (Aggregation operator)
    class OperatorElection: public SingleInputOperator {
        int window_width_;
        int current_window_position_;

        bool is_outputtable_() {
            return ++current_window_position_ % window_width_ == 0;
        }

    public:
        OperatorElection(Operator::ptr_t parent_operator_ptr, int window_width):
            SingleInputOperator(parent_operator_ptr),
            // save arguments as private member
            window_width_(window_width),
            current_window_position_(0) {
        }

        Tuple::ptr_t process_single_input(Tuple::ptr_t input_tuple) {
            if (is_outputtable_())
                output_tuple(input_tuple);
            return Tuple::ptr_t();
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_ELECTION_H_ */
