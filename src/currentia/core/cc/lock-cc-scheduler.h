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
        }

    protected:
        void commit_() {
            // Nothing
        }

        void after_commit_(time_t hwm) {
            // Release lock
            std::clog << "Release all locks!" << std::endl;
            release_all_locks_();
            // evict tuples!
            std::clog << "Now, evict tuples!" << std::endl;
            // Recover clean state
            evict_backup_tuples_(hwm);
            reset_operators_and_streams_();
        }

        void release_all_locks_() {
            auto iter = reference_operators_.begin();
            auto iter_end = reference_operators_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->transaction_end(Operator::PESSIMISTIC_2PL);
            }
        }
    };
}

#endif  /* ! CURRENTIA_LOCK_CC_SCHEDULER_H_ */
