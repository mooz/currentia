// -*- c++ -*-

#ifndef CURRENTIA_ABSTRACT_PESSIMISTIC_CC_SCHEDULER_H_
#define CURRENTIA_ABSTRACT_PESSIMISTIC_CC_SCHEDULER_H_

#include "currentia/core/cc/abstract-cc-scheduler.h"
#include "currentia/core/operator/operator.h"

namespace currentia {
    // Concurrenty Control Scheduler with 2-Phase Locking Protocol
    class AbstractPessimisticCCScheduler : public AbstractCCScheduler {
    protected:
        int txn_joint_count_;
        int commit_count_;

        bool commit_count_reached_limit_() {
            return commit_count_ >= txn_joint_count_;
        }

    public:
        AbstractPessimisticCCScheduler(Operator::ptr_t root_operator,
                                       Operator::CCMode cc_mode,
                                       int txn_joint_count = 1):
            AbstractCCScheduler(root_operator, cc_mode),
            txn_joint_count_(txn_joint_count),
            commit_count_(0) {
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
                    case TraitAggregationOperator::COMMIT:
                        commit_count_++;
                        commit_();
                        break;
                    default:
                        throw x;
                    }
                } catch (time_t hwm) {
                    if (commit_count_reached_limit_()) {
                        commit_count_ = 0;
                        after_commit_(hwm);
                    }
                }
            }
        }

    protected:
        virtual void commit_() = 0;
        virtual void after_commit_(time_t hwm) = 0;
    };
}

#endif  /* ! CURRENTIA_ABSTRACT_PESSIMISTIC_CC_SCHEDULER_H_ */
