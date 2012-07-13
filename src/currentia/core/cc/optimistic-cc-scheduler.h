// -*- c++ -*-

#ifndef CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_
#define CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_

#include "currentia/core/cc/abstract-cc-scheduler.h"

namespace currentia {
    class OptimisticCCScheduler : public AbstractCCScheduler {
        int redo_counts_;

    public:
        OptimisticCCScheduler(Operator::ptr_t root_operator):
            AbstractCCScheduler(root_operator, Operator::OPTIMISTIC),
            redo_counts_(0) {
        }

        void wake_up() {
            Operator* next_operator = get_next_operator_();
            if (next_operator != commit_operator_) {
                next_operator->process_next();
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
        }

        int get_redo_counts() {
            return redo_counts_;
        }

    private:
        void prepare_for_redo_() {
            ++redo_counts_;
            // Reset streams and operators only (not do eviction)
            reset_streams_();
            reset_operators_();
            current_operator_index_ = 0;
        }

        void commit_() {
            time_t lwm = dynamic_cast<TraitAggregationOperator*>(commit_operator_)
                         ->get_window_beginning_lwm();
            evict_backup_tuples_(lwm);
        }
    };
}

#endif  /* ! CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_ */
