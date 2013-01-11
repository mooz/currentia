// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_JOIN_H_
#define CURRENTIA_OPERATOR_JOIN_H_

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"
#include "currentia/core/window.h"

#include "currentia/core/operator/double-input-operator.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/synopsis.h"

#include <functional>

namespace currentia {
    class OperatorJoin: public DoubleInputOperator {
    public:
        OperatorJoin(const Operator::ptr_t& parent_left_operator_ptr,
                     Window left_window,
                     const Operator::ptr_t& parent_right_operator_ptr,
                     Window right_window,
                     Condition::ptr_t join_condition):
            DoubleInputOperator(parent_left_operator_ptr,
                                parent_right_operator_ptr),
            left_window_(left_window),
            right_window_(right_window),
            // init synopsises
            left_synopsis_(create_synopsis_from_window(left_window_)),
            right_synopsis_(create_synopsis_from_window(right_window_)),
            // set join condition
            join_condition_(join_condition) {
            // build new schema and index
            joined_schema_ptr_ = build_joined_schema_();
            set_output_stream(Stream::from_schema(joined_schema_ptr_));
            // set callbacks
            left_synopsis_->set_on_accept(std::bind(&OperatorJoin::left_on_accept_, this));
            right_synopsis_->set_on_accept(std::bind(&OperatorJoin::right_on_accept_, this));
            // obey schema
            join_condition->obey_schema(
                parent_left_operator_ptr->get_output_stream()->get_schema(),
                parent_right_operator_ptr->get_output_stream()->get_schema()
            );
        }

        void process_left_input(const Tuple::ptr_t& input) {
            left_synopsis_->enqueue(input);
        }

        void process_right_input(const Tuple::ptr_t& input) {
            right_synopsis_->enqueue(input);
        }

        void reset() {
            left_synopsis_->reset();
            right_synopsis_->reset();
        }

    private:
        Window left_window_;
        Window right_window_;

        Synopsis::ptr_t left_synopsis_;
        Synopsis::ptr_t right_synopsis_;

        Condition::ptr_t join_condition_;

        Schema::ptr_t joined_schema_ptr_;

        void left_on_accept_() {
            left_needs_tuple_ = false;
            try_join_();
        }

        void right_on_accept_() {
            right_needs_tuple_ = false;
            try_join_();
        }

        Schema::ptr_t build_joined_schema_() {
            return concat_schemas(
                parent_left_operator_ptr_->get_output_schema_ptr(),
                parent_right_operator_ptr_->get_output_schema_ptr()
            );
        }

        void try_join_() {
            if (!left_needs_tuple_ && !right_needs_tuple_) {
                join_synopsis_();
                left_needs_tuple_ = right_needs_tuple_ = true;
            }
        }

        inline void
        join_synopsis_() {
            Synopsis::const_iterator left_iter = left_synopsis_->begin();
            Synopsis::const_iterator right_iter = right_synopsis_->begin();

            Synopsis::const_iterator left_iter_end = left_synopsis_->end();
            Synopsis::const_iterator right_iter_end = right_synopsis_->end();

#if 0
            std::cout << "Left Synopsis\n" << left_synopsis_->toString() << std::endl;
            std::cout << "Right Synopsis\n" << right_synopsis_->toString() << std::endl;
            std::cout.flush();
#endif

#ifdef CURRENTIA_ENABLE_TRANSACTION
            // decide lwm
            time_t lwm = std::min(left_synopsis_->get_lwm(),
                                  right_synopsis_->get_lwm());
#endif

            // For now, just conduct a nested-loop join
            for (; left_iter != left_iter_end; ++left_iter) {
                for (; right_iter != right_iter_end; ++right_iter) {
                    if (join_condition_->check(*left_iter, *right_iter)) {
                        auto combined_tuple = Tuple::create(
                            joined_schema_ptr_,
                            (*left_iter)->get_concatenated_data(*right_iter),
                            std::min((*left_iter)->get_arrived_time(), (*right_iter)->get_arrived_time())
                        );
#ifdef CURRENTIA_ENABLE_TRANSACTION
                        combined_tuple->set_lwm(lwm);
#endif
                        output_tuple(combined_tuple);
                    }
                }
            }
        }

    public:
        std::string toString() const {
            std::stringstream ss;
            ss << "(\n {\n  " << parent_left_operator_ptr_->toString()
               << "\n },\n {\n  " << parent_right_operator_ptr_->toString()
               << "\n }\n)\n -> " << get_name() << "(" << join_condition_->toString() << ")";
            return ss.str();
        }

        std::string get_name() const {
            return std::string("Join");
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_JOIN_H_ */
