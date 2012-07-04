// -*- c++ -*-

#ifndef CURRENTIA_CC_REDO_AREA_H_
#define CURRENTIA_CC_REDO_AREA_H_

#include "currentia/core/operator/double-input-operator.h"
#include "currentia/core/operator/operator-join.h"
#include "currentia/core/operator/operator-stream-adapter.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/trait-aggregation-operator.h"
#include "currentia/core/operator/trait-resource-reference-operator.h"
#include "currentia/core/pointer.h"

namespace currentia {
#define is_instance(name, klass) !!dynamic_cast<klass*>(name)

    class RedoArea : public Pointable<RedoArea> {
        Operator* root_operator_;
        std::deque<Stream::ptr_t> redo_streams_;
        std::deque<Operator*> operators_in_redo_area_;

    public:
        typedef Pointable<RedoArea>::ptr_t ptr_t;

        RedoArea(Operator* root_operator):
            root_operator_(root_operator) {
            init_status_();
            setup_redo_queue_(root_operator_);
        }

        std::deque<Stream::ptr_t> get_redo_streams() {
            return redo_streams_;
        }

        std::deque<Operator*> get_redo_operators() {
            return operators_in_redo_area_;
        }

    private:
        void init_status_() {
            redo_streams_.clear();
            operators_in_redo_area_.clear();
        }

        // returns
        bool setup_redo_queue_(Operator* op, bool downstream_has_window = false) {
            if (is_instance(op, OperatorStreamAdapter))
                return false;

            if (is_instance(op, SingleInputOperator)) {
                return setup_redo_queue_single_(dynamic_cast<SingleInputOperator*>(op),
                                                downstream_has_window);
            } else if (is_instance(op, DoubleInputOperator)) {
                return setup_redo_queue_double_(dynamic_cast<DoubleInputOperator*>(op),
                                                downstream_has_window);
            } else {
                return false;
            }
        }

        void make_operator_output_stream_backedup(const Operator::ptr_t& op) {
            make_operator_output_stream_backedup(op.get());
        }

        void make_operator_output_stream_backedup(Operator* op) {
            redo_streams_.push_back(op->get_output_stream());
#if 0
            std::clog << "Make operator " << op->get_name() << "'s input backup" << std::endl;
#endif
            op->get_output_stream()->set_backup_state(true);
        }

        void push_operator_into_redo_area(Operator* op) {
#if 0
            std::clog << "Push operator " << op->get_name() << " into redo area" << std::endl;
#endif
            operators_in_redo_area_.push_back(op);
        }

        bool is_window_operator(Operator* op) const {
            return is_instance(op, TraitAggregationOperator) || is_instance(op, OperatorJoin);
        }

        bool setup_redo_queue_single_(SingleInputOperator* op, bool downstream_has_window) {
            auto upstream_has_reference_operator = setup_redo_queue_(
                op->get_parent_operator().get(),
                downstream_has_window || is_window_operator(op)
            );

            if (is_instance(op, TraitResourceReferenceOperator) &&
                !upstream_has_reference_operator &&
                downstream_has_window) {
                make_operator_output_stream_backedup(op->get_parent_operator());
                upstream_has_reference_operator = true;
            }

            if (upstream_has_reference_operator &&
                (downstream_has_window || is_instance(op, TraitAggregationOperator)))
                push_operator_into_redo_area(op);

            return upstream_has_reference_operator;
        }

        bool setup_redo_queue_double_(DoubleInputOperator* op, bool downstream_has_window) {
            auto downstream_has_window_for_parent =
                downstream_has_window || is_window_operator(op);

            // Left and Right
            auto left_upstream_has_reference_operator = setup_redo_queue_(
                op->get_parent_left_operator().get(),
                downstream_has_window_for_parent
            );
            auto right_upstream_has_reference_operator = setup_redo_queue_(
                op->get_parent_right_operator().get(),
                downstream_has_window_for_parent
            );

            auto upstream_has_reference_operator = (left_upstream_has_reference_operator ||
                                                    right_upstream_has_reference_operator);

#if 0
            std::clog << "----------------------------" << std::endl;
            std::clog << "downstream_has_window_for_parent: "
                      << downstream_has_window_for_parent << std::endl;
            std::clog << "left_upstream_has_reference_operator: "
                      << left_upstream_has_reference_operator << std::endl;
            std::clog << "right_upstream_has_reference_operator: "
                      << right_upstream_has_reference_operator << std::endl;
            std::clog << "downstream_has_window_for_parent: "
                      << downstream_has_window_for_parent << std::endl;
#endif

            if (is_window_operator(op) &&
                upstream_has_reference_operator &&
                downstream_has_window_for_parent) {
                if (!left_upstream_has_reference_operator) {
                    make_operator_output_stream_backedup(op->get_parent_left_operator());
                } else if (!right_upstream_has_reference_operator) {
                    make_operator_output_stream_backedup(op->get_parent_right_operator());
                }
                push_operator_into_redo_area(op);
            }

            return upstream_has_reference_operator;
        }
    };

#undef is_instance
}

#endif  /* ! CURRENTIA_CC_REDO_AREA_H_ */
