// -*- c++ -*-

#ifndef CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_
#define CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_

#include "currentia/core/cc/abstract-cc-scheduler.h"

namespace currentia {
    class OptimisticCCScheduler : public AbstractCCScheduler {
        int redo_counts_;

    public:
        OptimisticCCScheduler(const Operator::ptr_t& root_operator,
                              const SchedulingPolicyFactory::ptr_t& scheduling_policy_factory):
            AbstractCCScheduler(root_operator, scheduling_policy_factory, Operator::OPTIMISTIC) {
        }

        bool wake_up() {
            Operator* next_operator = get_next_operator_();
            if (!next_operator)
                return false;
            if (next_operator != commit_operator_) {
                process_operator_batch_(next_operator);
            } else {
                try {
                    next_operator->process_next();
                } catch (TraitAggregationOperator::Message x) {
                    switch (x) {
                    case TraitAggregationOperator::LOST_CONSISTENCY:
                        prepare_for_redo_();
                        break;
                    case TraitAggregationOperator::COMMIT:
                        commit_();
                        break;
                    default:
                        throw x;
                    }
                }
            }
            return false;
        }

    private:
        void prepare_for_redo_() {
            // Reset streams and operators only (not do eviction)
            enter_redo_();
            reset_streams_();
            reset_operators_();
            scheduling_policy_->reset();
        }

        void commit_() {
            time_t lwm = dynamic_cast<TraitAggregationOperator*>(commit_operator_)
                         ->get_window_beginning_lwm();
            evict_backup_tuples_(lwm);
        }
    };
}

#endif  /* ! CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_ */
