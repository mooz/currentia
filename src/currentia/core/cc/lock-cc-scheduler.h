// -*- c++ -*-

#ifndef CURRENTIA_LOCK_CC_SCHEDULER_H_
#define CURRENTIA_LOCK_CC_SCHEDULER_H_

#include "currentia/core/cc/abstract-pessimistic-cc-scheduler.h"

namespace currentia {
    // Concurrenty Control Scheduler with 2-Phase Locking Protocol
    class LockCCScheduler : public AbstractPessimisticCCScheduler {
    public:
        LockCCScheduler(const Operator::ptr_t& root_operator,
                        const SchedulingPolicyFactory::ptr_t& scheduling_policy_factory,
                        int txn_joint_count = 1):
            AbstractPessimisticCCScheduler(root_operator,
                                           scheduling_policy_factory,
                                           Operator::PESSIMISTIC_2PL,
                                           txn_joint_count) {
        }

        ~LockCCScheduler() {
            release_all_locks_();
        }

    protected:
        void commit_() {
            // Nothing
        }

        void after_commit_(time_t lwm) {
            release_all_locks_();
            if (commit_operator_overlaps_) {
                evict_and_reset_streams_(lwm);
                reset_operators_();
            }
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
