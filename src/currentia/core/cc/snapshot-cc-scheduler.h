// -*- c++ -*-

#ifndef CURRENTIA_SNAPSHOT_CC_SCHEDULER_H_
#define CURRENTIA_SNAPSHOT_CC_SCHEDULER_H_

#include "currentia/core/cc/abstract-pessimistic-cc-scheduler.h"

namespace currentia {
    // Concurrenty Control Scheduler with 2-Phase Locking Protocol
    class SnapshotCCScheduler : public AbstractPessimisticCCScheduler {
    public:
        SnapshotCCScheduler(Operator::ptr_t root_operator,
                            const SchedulingPolicyFactory::ptr_t& scheduling_policy_factory,
                            int txn_joint_count = 1):
            AbstractPessimisticCCScheduler(root_operator,
                                           scheduling_policy_factory,
                                           Operator::PESSIMISTIC_SNAPSHOT,
                                           txn_joint_count) {
        }

    protected:
        void commit_() {
            // Nothing
        }

        void after_commit_(time_t lwm) {
            refresh_snapshots_();
            evict_and_reset_streams_(lwm);
            reset_operators_();
        }

        void refresh_snapshots_() {
            auto iter = reference_operators_.begin();
            auto iter_end = reference_operators_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->transaction_end(Operator::PESSIMISTIC_SNAPSHOT);
            }
        }
    };
}

#endif  /* ! CURRENTIA_SNAPSHOT_CC_SCHEDULER_H_ */
