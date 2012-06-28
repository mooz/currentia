// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_AGGREGATION_H_
#define CURRENTIA_OPERATOR_AGGREGATION_H_

#include "currentia/core/window.h"
#include "currentia/core/operator/synopsis.h"
#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/visitable-operator.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class AggregationOperator {
    protected:
        Window window_;
        Synopsis synopsis_;

        AggregationOperator(Window window,
                            const Synopsis::callback_t& on_accept):
            window_(window),
            synopsis_(window) {
            // Setup handler
            synopsis_.set_on_accept(on_accept);
        }

        

        virtual ~AggregationOperator() = 0;
    };

    AggregationOperator::~AggregationOperator() {}
}

#endif  /* ! CURRENTIA_OPERATOR_AGGREGATION_H_ */
