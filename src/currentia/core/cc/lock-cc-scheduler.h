// -*- c++ -*-

#ifndef CURRENTIA_LOCK_CC_SCHEDULER_H_
#define CURRENTIA_LOCK_CC_SCHEDULER_H_

#include "currentia/core/cc/abstract-cc-scheduler.h"
#include "currentia/core/cc/trait-pessimistic-cc.h"

namespace currentia {
    // Concurrenty Control Scheduler with 2-Phase Locking Protocol
    class LockCCScheduler : public AbstractCCScheduler,
                            public TraitPessimisticCC {
    public:
        LockCCScheduler(Operator::ptr_t root_operator, int txn_joint_count = 1):
            AbstractCCScheduler(root_operator, Operator::PESSIMISTIC_2PL),
            TraitPessimisticCC(txn_joint_count) {
            // for each operator in redo_area
            //   set cc_mode to "2PL"
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
                        // ARIENAI!
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

    private:
        void commit_() {
            time_t hwm = dynamic_cast<TraitAggregationOperator*>(commit_operator_)
                         ->get_window_beginning_hwm();
            // std::clog << "Commit! Evict tuples older than " << hwm << std::endl;
            evict_backup_tuples_(hwm);
        }
    };
}

#endif  /* ! CURRENTIA_LOCK_CC_SCHEDULER_H_ */
