// -*- c++ -*-

#ifndef CURRENTIA_LOCK_CC_SCHEDULER_H_
#define CURRENTIA_LOCK_CC_SCHEDULER_H_

#include "currentia/core/cc/abstract-pessimistic-cc-scheduler.h"

namespace currentia {
    // Concurrenty Control Scheduler with 2-Phase Locking Protocol
    class LockCCScheduler : public AbstractPessimisticCCScheduler {
    public:
        LockCCScheduler(Operator::ptr_t root_operator, int txn_joint_count = 1):
            AbstractPessimisticCCScheduler(root_operator,
                                           Operator::PESSIMISTIC_2PL,
                                           txn_joint_count) {
            // for each operator in redo_area
            //   set cc_mode to "2PL"
        }

    protected:
        void commit_() {
            // Nothing
        }

        void after_commit_(time_t hwm) {
            // evict tuples!
            std::clog << "Now, evict tuples!" << std::endl;
            // Release lock

            // Recover clean state
            evict_backup_tuples_(hwm);
            reset_operators_and_streams_();
        }
    };
}

#endif  /* ! CURRENTIA_LOCK_CC_SCHEDULER_H_ */
