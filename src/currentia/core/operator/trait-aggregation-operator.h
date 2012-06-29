// -*- c++ -*-

#ifndef CURRENTIA_TRAIT_AGGREGATION_OPERATOR_H_
#define CURRENTIA_TRAIT_AGGREGATION_OPERATOR_H_

#include "currentia/core/window.h"
#include "currentia/core/operator/synopsis.h"
#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/visitable-operator.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class TraitAggregationOperator {
    public:
        enum Error {
            LOST_CONSISTENCY
        };

    protected:
        Window window_;
        Synopsis synopsis_;

        Synopsis::callback_t on_accept_;

        TraitAggregationOperator(Window window,
                                 const Synopsis::callback_t& on_accept):
            window_(window),
            synopsis_(window),
            on_accept_(on_accept) {
            // Setup handler
            synopsis_.set_on_accept(std::bind(&TraitAggregationOperator::on_accept_wrapper_, this));
        }

        virtual ~TraitAggregationOperator() = 0;

    private:
        void on_accept_wrapper_() {
            std::cout << synopsis_.has_reference_consistency() << std::endl;

            if (synopsis_.has_reference_consistency())
                on_accept_();
            else
                throw LOST_CONSISTENCY;
        }
    };

    TraitAggregationOperator::~TraitAggregationOperator() {}
}

#endif  /* ! CURRENTIA_TRAIT_AGGREGATION_OPERATOR_H_ */
