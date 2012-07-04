// -*- c++ -*-

#ifndef CURRENTIA_CC_COMMIT_OPERATOR_FINDER_H_
#define CURRENTIA_CC_COMMIT_OPERATOR_FINDER_H_

#include "currentia/core/operator/double-input-operator.h"
#include "currentia/core/operator/operator-stream-adapter.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/trait-aggregation-operator.h"
#include "currentia/core/operator/trait-resource-reference-operator.h"
#include "currentia/core/pointer.h"

namespace currentia {
#define is_instance(name, klass) !!dynamic_cast<klass*>(name)

    class CommitOperatorFinder : public Pointable<CommitOperatorFinder> {
        Operator* root_operator_;
        Operator* commit_operator_;

        Operator* current_candidate_;
        int reference_operator_count_under_candidate_;


    public:
        typedef Pointable<CommitOperatorFinder>::ptr_t ptr_t;

        CommitOperatorFinder(Operator* root_operator):
            root_operator_(root_operator),
            commit_operator_(NULL) {
        }

        Operator* get_commit_operator() {
            if (commit_operator_ == NULL) {
                init_find_status_();
                find_commit_operator_(root_operator_);
                commit_operator_ = current_candidate_;
            }
            return commit_operator_;
        }

    private:
        void init_find_status_() {
            current_candidate_ = NULL;
            reference_operator_count_under_candidate_ = 0;
        }

        // returns the number of reference nodes in the sub-tree rooted from <op>
        int find_commit_operator_(Operator* op) {
            // DFS:
            //   Traverse children first.
            //   Get a commit operator candidate <cand>.
            // Visit (post-order):
            //   Record window operator as the first commit operator.

            if (is_instance(op, SingleInputOperator)) {
                return find_commit_operator_single_(dynamic_cast<SingleInputOperator*>(op));
            } else if (is_instance(op, DoubleInputOperator)) {
                return find_commit_operator_double_(dynamic_cast<DoubleInputOperator*>(op));
            } else {
                return 0;
            }
        }

        int find_commit_operator_single_(SingleInputOperator* op) {
            auto child_reference_count = find_commit_operator_(op->get_parent_operator().get());

            if (is_instance(op, TraitResourceReferenceOperator)) {
                std::clog << "Found reference operator!" << std::endl;
                return child_reference_count + 1;
            } else if (is_instance(op, TraitAggregationOperator) &&
                       child_reference_count > reference_operator_count_under_candidate_) {
                // This node is a commit operator candidate
                current_candidate_ = op;
                reference_operator_count_under_candidate_ = child_reference_count;
                return child_reference_count;
            }

            return child_reference_count;
        }

        int find_commit_operator_double_(DoubleInputOperator* op) {
            // Left and Right
            auto left_child_reference_count = find_commit_operator_(op->get_parent_left_operator().get());
            auto right_child_reference_count = find_commit_operator_(op->get_parent_right_operator().get());

            if (left_child_reference_count > 0 && right_child_reference_count > 0) {
                current_candidate_ = op;
                reference_operator_count_under_candidate_ = left_child_reference_count + right_child_reference_count;
            }

            return left_child_reference_count + right_child_reference_count;
        }
    };

#undef is_instance
}

#endif  /* ! CURRENTIA_CC_COMMIT_OPERATOR_FINDER_H_ */
