// -*- c++ -*-

#ifndef CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_
#define CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_

#include "currentia/core/operator/operator-visitor-serializer.h"
#include "currentia/core/operator/trait-aggregation-operator.h"
#include "currentia/core/operator/operator-mean.h"
#include "currentia/core/operator/trait-resource-reference-operator.h"
#include "currentia/core/cc/abstract-cc-scheduler.h"
#include "currentia/core/cc/commit-operator-finder.h"
#include "currentia/core/cc/redo-area.h"

#include "currentia/core/operator/dump-operator-tree.h"
#include "currentia/util/print.h"

namespace currentia {
    class OptimisticCCScheduler : public AbstractCCScheduler{
        int redo_counts_;

    public:
        OptimisticCCScheduler(Operator::ptr_t root_operator):
            AbstractCCScheduler(root_operator),
            redo_counts_(0) {
        }

        void wake_up() {
            Operator* next_operator = get_next_operator_();
            // std::clog << "Next operator -> " << next_operator->get_name() << std::endl;
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
            // std::clog << "Done!" << std::endl;
        }

        int get_redo_counts() {
            return redo_counts_;
        }

    private:
        void prepare_for_redo_() {
            ++redo_counts_;
            std::clog << "Redo: " << redo_counts_ << " times" << std::endl;
            reset_operators_();
            reset_streams_();
            current_operator_index_ = 0;
        }

        void commit_() {
            time_t hwm = dynamic_cast<TraitAggregationOperator*>(commit_operator_)
                         ->get_window_beginning_hwm();
            // std::clog << "Commit! Evict tuples older than " << hwm << std::endl;
            evict_backup_tuples_(hwm);
        }
    };
}

#endif  /* ! CURRENTIA_OPTIMISTIC_CC_SCHEDULER_H_ */
