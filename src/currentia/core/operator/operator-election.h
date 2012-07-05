// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_ELECTION_H_
#define CURRENTIA_OPERATOR_ELECTION_H_

#include "currentia/core/tuple.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/trait-aggregation-operator.h"

#include <functional>

namespace currentia {
    // Election (Aggregation operator)
    class OperatorElection: public SingleInputOperator,
                            public TraitAggregationOperator {
    public:
        OperatorElection(Operator::ptr_t parent_operator_ptr, Window window):
            SingleInputOperator(parent_operator_ptr),
            TraitAggregationOperator(window,
                                     std::bind(&OperatorElection::elect_tuple_, this)) {
            set_output_stream(Stream::from_schema(parent_operator_ptr->get_output_schema_ptr()));
        }

        void process_single_input(Tuple::ptr_t input_tuple) {
            output_tuple(input_tuple);
        }

        std::string toString() const {
            return parent_operator_ptr_->toString() + std::string(" -> ") + get_name();
        }

        std::string get_name() const {
            return std::string("Election");
        }

        void reset() {
            synopsis_.reset();
        }

    private:
        void elect_tuple_() {
            output_tuple(*synopsis_.begin());
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_ELECTION_H_ */
