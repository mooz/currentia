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
        enum Message {
            LOST_CONSISTENCY,
            COMMIT,
        };

    protected:
        Window window_;
        Synopsis::ptr_t synopsis_;

        Synopsis::callback_t on_accept_;

    public:
        TraitAggregationOperator(Window window,
                                 const Synopsis::callback_t& on_accept):
            window_(window),
            synopsis_(create_synopsis_from_window(window)),
            on_accept_(on_accept) {
            // Setup handler
            synopsis_->set_on_accept(std::bind(&TraitAggregationOperator::on_accept_wrapper_, this));
        }

        virtual ~TraitAggregationOperator() = 0;

        time_t get_window_beginning_lwm() const {
            // print_synopsis_lwm();
            return synopsis_->get_window_beginning_tuple()->get_lwm();
        }

        void print_synopsis_lwm() const {
            auto iter = synopsis_->begin();
            auto iter_end = synopsis_->end();
            for (; iter != iter_end; ++iter) {
                std::clog << (*iter)->get_lwm() << ", ";
            }
            std::clog << std::endl;
        }

        Window get_window() const {
            return window_;
        }

    private:
        void on_accept_wrapper_() {
            on_accept_();
        }
    };

    TraitAggregationOperator::~TraitAggregationOperator() {}
}

#endif  /* ! CURRENTIA_TRAIT_AGGREGATION_OPERATOR_H_ */
