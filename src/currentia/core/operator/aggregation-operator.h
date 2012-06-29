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

        Synopsis::callback_t on_accept_;

        AggregationOperator(Window window,
                            const Synopsis::callback_t& on_accept):
            window_(window),
            synopsis_(window),
            on_accept_(on_accept) {
            // Setup handler
            synopsis_.set_on_accept(std::bind(&AggregationOperator::on_accept_wrapper_, this));
        }

        virtual ~AggregationOperator() = 0;

    private:
        void on_accept_wrapper_() {
            if (synopsis_.has_reference_consistency())
                on_accept_();
            else
                throw std::string("Consistency Failure");
        }
    };

    AggregationOperator::~AggregationOperator() {}
}

#endif  /* ! CURRENTIA_OPERATOR_AGGREGATION_H_ */
