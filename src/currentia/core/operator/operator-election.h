// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_ELECTION_H_
#define CURRENTIA_OPERATOR_ELECTION_H_

#include "currentia/core/tuple.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/aggregation-operator.h"

#include <tr1/functional>

namespace currentia {
    // Election (Aggregation operator)
    class OperatorElection: public SingleInputOperator,
                            public AggregationOperator {
    public:
        OperatorElection(Operator::ptr_t parent_operator_ptr, Window window):
            SingleInputOperator(parent_operator_ptr),
            AggregationOperator(window,
                                std::tr1::bind(&OperatorElection::elect_tuple_, this)) {
            set_output_stream(Stream::from_schema(parent_operator_ptr->get_output_schema_ptr()));
        }

        void process_single_input(Tuple::ptr_t input_tuple) {
            output_tuple(input_tuple);
        }

        virtual std::string toString() const {
            return parent_operator_ptr_->toString() + std::string(" -> Election");
        }

    private:
        void elect_tuple_() {
            output_tuple(*synopsis_.begin());
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_ELECTION_H_ */
