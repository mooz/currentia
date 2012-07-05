// -*- c++ -*-

#ifndef CURRENTIA_ROUND_ROBIN_SCHEDULER_H_
#define CURRENTIA_ROUND_ROBIN_SCHEDULER_H_

#include "currentia/core/operator/operator-visitor-serializer.h"
#include "currentia/core/operator/trait-aggregation-operator.h"
#include "currentia/core/operator/trait-resource-reference-operator.h"
#include "currentia/core/scheduler/abstract-scheduler.h"

namespace currentia {
    class RoundRobinScheduler : public AbstractScheduler {
        OperatorVisitorSerializer serializer_;
        std::vector<Operator*> operators_;
        int current_operator_index_;

    public:
        RoundRobinScheduler(Operator::ptr_t root_operator):
            AbstractScheduler(root_operator),
            current_operator_index_(0) {
            serializer_.dispatch(root_operator.get());
            operators_ = serializer_.get_sorted_operators();
        }

        static bool is_aggregation_operator(const Operator* op) {
            return !!dynamic_cast<const TraitAggregationOperator*>(op);
        }

        static bool is_resource_reference_operator(const Operator* op) {
            return !!dynamic_cast<const TraitResourceReferenceOperator*>(op);
        }

        void wake_up() {
            Operator* next_operator = get_next_operator_();
            try {
                next_operator->process_next();
            } catch (TraitAggregationOperator::Error x) {
                if (x == TraitAggregationOperator::LOST_CONSISTENCY)
                    std::cout << "Consistency Failure" << std::endl;
                else
                    throw x;
            }
        }

    private:
        Operator* get_next_operator_() {
            return operators_[get_next_operator_index_()];
        }

        int get_next_operator_index_() {
            return current_operator_index_++ % operators_.size();
        }

        void reset_operators_() {
            auto iter = redo_operators_.begin();
            auto iter_end = redo_operators_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->reset();
                if (*iter != commit_operator_) {
                    (*iter)->get_output_stream()->clear();
                }
            }
        }
    };
}

#endif  /* ! CURRENTIA_ROUND_ROBIN_SCHEDULER_H_ */
