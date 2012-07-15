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

#include "currentia/core/operator/dump-operator-tree.h"
#include "currentia/util/print.h"

namespace currentia {
    // Concurrenty Control Scheduler
    class AbstractCCScheduler : public AbstractScheduler {
    protected:

        OperatorVisitorSerializer serializer_;
        std::vector<Operator*> operators_;
        int current_operator_index_;

        // Concurrency Control
        Operator* commit_operator_;
        std::deque<Operator*> redo_operators_;
        std::deque<Stream::ptr_t> redo_streams_;

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
        // @Changed
        AbstractCCScheduler(Operator::ptr_t root_operator, Operator::CCMode cc_mode):
            AbstractScheduler(root_operator),
            current_operator_index_(0) {
            serializer_.dispatch(root_operator.get());
            operators_ = serializer_.get_sorted_operators();
            std::for_each(operators_.begin(), operators_.end(), SetCCMode(cc_mode));

            // Concurrency Control
            CommitOperatorFinder finder(root_operator.get());
            commit_operator_ = finder.get_commit_operator();
            commit_operator_->set_is_commit_operator(true);
            if (!dynamic_cast<OperatorMean*>(commit_operator_))
                throw "Commit operator can only be OperatorMean for now";

            RedoArea redo_area(root_operator.get());
            redo_operators_ = redo_area.get_redo_operators();
            redo_streams_ = redo_area.get_redo_streams();

            util::print_iterable(redo_operators_);
            util::print_iterable(redo_streams_);
            std::clog << dump_operator_tree(root_operator.get()) << std::endl;
        }

        virtual ~AbstractCCScheduler() = 0;

        static bool is_aggregation_operator(const Operator* op) {
            return !!dynamic_cast<const TraitAggregationOperator*>(op);
        }

        static bool is_resource_reference_operator(const Operator* op) {
            return !!dynamic_cast<const TraitResourceReferenceOperator*>(op);
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
