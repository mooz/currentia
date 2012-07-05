// -*- c++ -*-

#ifndef CURRENTIA_ROUND_ROBIN_SCHEDULER_H_
#define CURRENTIA_ROUND_ROBIN_SCHEDULER_H_

#include "currentia/core/operator/operator-visitor-serializer.h"
#include "currentia/core/operator/trait-aggregation-operator.h"
#include "currentia/core/operator/operator-mean.h"
#include "currentia/core/operator/trait-resource-reference-operator.h"
#include "currentia/core/scheduler/abstract-scheduler.h"
#include "currentia/core/cc/commit-operator-finder.h"
#include "currentia/core/cc/redo-area.h"

namespace currentia {
    class RoundRobinScheduler : public AbstractScheduler {
        OperatorVisitorSerializer serializer_;
        std::vector<Operator*> operators_;
        int current_operator_index_;

        // Concurrency Control
        Operator* commit_operator_;
        std::deque<Operator*> redo_operators_;
        std::deque<Stream::ptr_t> redo_streams_;

        int redo_counts_;

    public:
        RoundRobinScheduler(Operator::ptr_t root_operator):
            AbstractScheduler(root_operator),
            current_operator_index_(0),
            redo_counts_(0) {
            serializer_.dispatch(root_operator.get());
            operators_ = serializer_.get_sorted_operators();

            // Concurrency Control
            CommitOperatorFinder finder(root_operator.get());
            commit_operator_ = finder.get_commit_operator();
            commit_operator_->set_is_commit_operator(true);
            if (!dynamic_cast<OperatorMean*>(commit_operator_))
                throw "Commit operator can only be OperatorMean for now";

            RedoArea redo_area(root_operator.get());
            redo_operators_ = redo_area.get_redo_operators();
            redo_streams_ = redo_area.get_redo_streams();
        }

        static bool is_aggregation_operator(const Operator* op) {
            return !!dynamic_cast<const TraitAggregationOperator*>(op);
        }

        static bool is_resource_reference_operator(const Operator* op) {
            return !!dynamic_cast<const TraitResourceReferenceOperator*>(op);
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
                    case TraitAggregationOperator::LOST_CONSISTENCY:
                        prepare_for_redo_();
                        break;
                    case TraitAggregationOperator::COMMIT:
                        commit_();
                        break;
                    default:
                        throw x;
                    }
                }
            }
        int get_redo_counts() {
            return redo_counts_;
        }

    private:
        Operator* get_next_operator_() {
            return operators_[get_next_operator_index_()];
        }

        int get_next_operator_index_() {
            return current_operator_index_++ % operators_.size();
        }

        void prepare_for_redo_() {
            ++redo_counts_;
            reset_operators_();
            reset_streams_();
            current_operator_index_ = 0;
        }

        void commit_() {
            time_t hwm = dynamic_cast<TraitAggregationOperator*>(commit_operator_)->get_window_beginning_hwm();
            std::clog << "Commit! Evict tuples older than " << hwm << std::endl;
            evict_backup_tuples_(hwm);
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

        void evict_backup_tuples_(int hwm) {
            // eviction
            auto iter = redo_streams_.begin();
            auto iter_end = redo_streams_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->evict_backup_tuples_older_than(hwm);
            }
        }
    };
}

#endif  /* ! CURRENTIA_ROUND_ROBIN_SCHEDULER_H_ */
