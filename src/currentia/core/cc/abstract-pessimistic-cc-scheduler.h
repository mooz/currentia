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

        std::deque<TraitResourceReferenceOperator*> reference_operators_;

    public:
        AbstractPessimisticCCScheduler(Operator::ptr_t root_operator,
                                       Operator::CCMode cc_mode,
                                       int txn_joint_count = 1):
            AbstractCCScheduler(root_operator, cc_mode),
            txn_joint_count_(txn_joint_count),
            commit_count_(0) {
            // Extract operator relation-join
            auto iter = operators_.begin();
            auto iter_end = operators_.end();
            for (; iter != iter_end; ++iter) {
                TraitResourceReferenceOperator* op =
                    dynamic_cast<TraitResourceReferenceOperator*>(*iter);
                if (op) {
                    reference_operators_.push_back(op);
                }
            }
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
                } catch (time_t lwm) {
                    if (commit_count_reached_limit_()) {
                        commit_count_ = 0;
                        after_commit_(lwm);
                    }
                }
            }
        }

    protected:
        virtual void commit_() = 0;
        virtual void after_commit_(time_t lwm) = 0;

        void evict_and_reset_streams_(time_t lwm) {
            {
                // Lock all streams
                auto iter = redo_streams_.begin();
                auto iter_end = redo_streams_.end();
                for (; iter != iter_end; ++iter) {
                    (*iter)->lock();
                }
            }
            evict_backup_tuples_(lwm);
            reset_streams_();
            {
                // Unlock all streams
                auto iter = redo_streams_.begin();
                auto iter_end = redo_streams_.end();
                for (; iter != iter_end; ++iter) {
                    (*iter)->unlock();
                }
            }
        }
    };
}

#endif  /* ! CURRENTIA_ABSTRACT_PESSIMISTIC_CC_SCHEDULER_H_ */
