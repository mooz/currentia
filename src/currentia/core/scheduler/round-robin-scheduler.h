// -*- c++ -*-

#ifndef CURRENTIA_ROUND_ROBIN_SCHEDULER_H_
#define CURRENTIA_ROUND_ROBIN_SCHEDULER_H_

#include "currentia/core/operator/operator-mean.h"
#include "currentia/core/operator/operator-visitor-serializer.h"
#include "currentia/core/scheduler/abstract-scheduler.h"
#include "currentia/core/cc/commit-operator-finder.h"

namespace currentia {
    // Concurrenty Control Scheduler
    class RoundRobinScheduler : public AbstractScheduler {
    protected:

        OperatorVisitorSerializer serializer_;
        std::vector<Operator*> operators_;
        int current_operator_index_;

        Operator* commit_operator_;

        // Functor
        struct SetCCMode {
            Operator::CCMode cc_mode;
            SetCCMode(Operator::CCMode cc_mode): cc_mode(cc_mode) {
            }
            void operator()(Operator* op) {
                op->set_cc_mode(cc_mode);
            }
        };

    public:
        RoundRobinScheduler(Operator::ptr_t root_operator):
            AbstractScheduler(root_operator),
            current_operator_index_(0) {
            serializer_.dispatch(root_operator.get());
            operators_ = serializer_.get_sorted_operators();
            std::for_each(operators_.begin(), operators_.end(), SetCCMode(Operator::NONE));

            CommitOperatorFinder finder(root_operator.get());
            commit_operator_ = finder.get_commit_operator();
            commit_operator_->set_is_commit_operator(true);
            if (!dynamic_cast<OperatorMean*>(commit_operator_))
                throw "Commit operator can only be OperatorMean for now";
        }

        void wake_up() {
            get_next_operator_()->process_next();
        }

        double get_consistent_rate() const {
            return dynamic_cast<OperatorMean*>(commit_operator_)->get_consistent_rate();
        }

        Operator* get_commit_operator() const {
            return commit_operator_;
        }

    protected:
        Operator* get_next_operator_() {
            return operators_[get_next_operator_index_()];
        }

        int get_next_operator_index_() {
            return current_operator_index_++ % operators_.size();
        }
    };
}

#endif  /* ! CURRENTIA_ROUND_ROBIN_SCHEDULER_H_ */
