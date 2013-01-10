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
        Operator* commit_operator_;

    public:
        typedef Pointable<CommitOperatorFinder>::ptr_t ptr_t;

        CommitOperatorFinder(Operator* root_operator):
            commit_operator_(NULL) {
            init_find_status_();
            find_commit_operator_(root_operator);
        }

        Operator* get_commit_operator() {
            return commit_operator_;
        }

    private:
        void init_find_status_() {
            commit_operator_ = NULL;
        }

        // returns the number of reference nodes in the sub-tree rooted from <op>
        bool find_commit_operator_(Operator* op) {
            if (is_instance(op, SingleInputOperator)) {
                return find_commit_operator_single_(dynamic_cast<SingleInputOperator*>(op));
            } else if (is_instance(op, DoubleInputOperator)) {
                return find_commit_operator_double_(dynamic_cast<DoubleInputOperator*>(op));
            } else {
                return false;
            }
        }

        bool find_commit_operator_single_(SingleInputOperator* op) {
            auto upstream_has_reference_operator = find_commit_operator_(op->get_parent_operator().get());

            if (is_instance(op, TraitResourceReferenceOperator)) {
                upstream_has_reference_operator = true;
            } else if (is_instance(op, TraitAggregationOperator) &&
                       upstream_has_reference_operator) {
                // This node is a commit operator candidate
                commit_operator_ = op;
            }

            return upstream_has_reference_operator;
        }

        bool find_commit_operator_double_(DoubleInputOperator* op) {
            // Left and Right
            auto left_upstream_has_reference_operator = find_commit_operator_(op->get_parent_left_operator().get());
            auto right_upstream_has_reference_operator = find_commit_operator_(op->get_parent_right_operator().get());

            if (left_upstream_has_reference_operator ||
                right_upstream_has_reference_operator) {
                commit_operator_ = op;
            }

            return left_upstream_has_reference_operator || right_upstream_has_reference_operator;
        }
    };

#undef is_instance
}

#endif  /* ! CURRENTIA_CC_COMMIT_OPERATOR_FINDER_H_ */
