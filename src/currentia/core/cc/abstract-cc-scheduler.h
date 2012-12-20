// -*- c++ -*-

#ifndef CURRENTIA_ABSTRACT_CC_SCHEDULER_H_
#define CURRENTIA_ABSTRACT_CC_SCHEDULER_H_

#include "currentia/core/operator/operator-visitor-serializer.h"
#include "currentia/core/operator/trait-aggregation-operator.h"
#include "currentia/core/operator/operator-mean.h"
#include "currentia/core/operator/trait-resource-reference-operator.h"
#include "currentia/core/scheduler/abstract-scheduler.h"
#include "currentia/core/cc/commit-operator-finder.h"
#include "currentia/core/cc/redo-area.h"

#include "currentia/core/scheduler/policy/scheduling-policy.h"

#include "currentia/core/operator/dump-operator-tree.h"
#include "currentia/util/print.h"

namespace currentia {
    // Concurrenty Control Scheduler
    class AbstractCCScheduler : public AbstractScheduler {
    protected:
        Operator* commit_operator_;
        std::deque<Operator*> redo_operators_;
        std::deque<Stream::ptr_t> redo_streams_;

    public:
        AbstractCCScheduler(const Operator::ptr_t& root_operator,
                            const SchedulingPolicyFactory::ptr_t& scheduling_policy_factory,
                            Operator::CCMode cc_mode):
            AbstractScheduler(root_operator, scheduling_policy_factory) {
            // Tell concurrency-control stragety to the operators rooted by `root_operator`
            for (auto iter = operators_.begin(), iter_end = operators_.end();
                 iter != iter_end;
                 ++iter) {
                (*iter)->set_cc_mode(cc_mode);
            }

            Operator* root_operator_raw_ptr = root_operator.get();

            // Find commit operator
            CommitOperatorFinder finder(root_operator_raw_ptr);
            commit_operator_ = finder.get_commit_operator();
            commit_operator_->set_is_commit_operator(true);
            if (!dynamic_cast<OperatorMean*>(commit_operator_))
                throw "Commit operator can only be OperatorMean for now";

            // Find redo area and redo stream
            RedoArea redo_area(root_operator_raw_ptr);
            redo_operators_ = redo_area.get_redo_operators();
            redo_streams_ = redo_area.get_redo_streams();

#ifdef CURRENTIA_DEBUG
            util::print_iterable(redo_operators_);
            util::print_iterable(redo_streams_);
            std::clog << dump_operator_tree(root_operator_raw_ptr) << std::endl;
#endif
        }

        virtual ~AbstractCCScheduler() = 0;

        double get_consistent_rate() const {
            return dynamic_cast<OperatorMean*>(commit_operator_)->get_consistent_rate();
        }

        Operator* get_commit_operator() const {
            return commit_operator_;
        }

    protected:
        Operator* get_next_operator_() const {
            return scheduling_policy_->get_next_operator();
        }

        void reset_operators_() {
            auto iter = redo_operators_.begin();
            auto iter_end = redo_operators_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->reset();
                if (*iter != commit_operator_) {
                    (*iter)->get_output_stream()->clear();
                }
            }
        }

        void reset_streams_() {
            auto iter = redo_streams_.begin();
            auto iter_end = redo_streams_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->clear();
                (*iter)->recover_from_backup();
            }
        }

        void evict_backup_tuples_(int lwm) {
            // eviction
            auto iter = redo_streams_.begin();
            auto iter_end = redo_streams_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->evict_backup_tuples_older_than(lwm);
            }
        }
    };

    AbstractCCScheduler::~AbstractCCScheduler() {}
}

#endif  /* ! CURRENTIA_ABSTRACT_CC_SCHEDULER_H_ */
