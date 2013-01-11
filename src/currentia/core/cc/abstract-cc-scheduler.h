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

        long reset_tuples_count_;
        long redo_counts_;

    public:
        AbstractCCScheduler(const Operator::ptr_t& root_operator,
                            const SchedulingPolicyFactory::ptr_t& scheduling_policy_factory,
                            Operator::CCMode cc_mode):
            AbstractScheduler(root_operator, scheduling_policy_factory),
            reset_tuples_count_(0),
            redo_counts_(0) {
            // Tell concurrency-control stragety to the operators rooted by `root_operator`
            for (auto iter = operators_.begin(), iter_end = operators_.end();
                 iter != iter_end;
                 ++iter) {
                (*iter)->set_cc_mode(cc_mode);
            }

            Operator* root_operator_raw_ptr = root_operator.get();

            // Find commit operator
            commit_operator_ = CommitOperatorFinder::find_commit_operator(root_operator_raw_ptr);
            if (commit_operator_) {
                commit_operator_->set_is_commit_operator(true);
            }

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
            assert(commit_operator_ != NULL);
            return dynamic_cast<TraitAggregationOperator*>(commit_operator_)->get_consistent_rate();
        }

        Operator* get_commit_operator() const {
            return commit_operator_;
        }

#ifdef CURRENTIA_CHECK_STATISTICS
        long get_redo_counts() {
            return redo_counts_;
        }

        long get_reset_tuples_count() const {
            return reset_tuples_count_;
        }

        long get_total_evaluation_count() const {
            long total_evaluation_count = 0;
            for (auto iter = operators_.begin(), iter_end = operators_.end();
                 iter != iter_end;
                 ++iter) {
                total_evaluation_count += (*iter)->get_evaluation_count();
            }
            return total_evaluation_count;
        }
#endif

    protected:
        void enter_redo_() {
#ifdef CURRENTIA_CHECK_STATISTICS
            ++redo_counts_;
#endif
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
#ifdef CURRENTIA_CHECK_STATISTICS
                reset_tuples_count_ += (*iter)->get_tuples_count();
#endif
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
