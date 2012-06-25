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

#include <tr1/functional>

namespace currentia {
    class OperatorJoin: public DoubleInputOperator {
    public:
        OperatorJoin(Operator::ptr_t parent_left_operator_ptr,
                     Window left_window,
                     Operator::ptr_t parent_right_operator_ptr,
                     Window right_window,
                     ConditionAttributeComparator::ptr_t attribute_comparator):
            DoubleInputOperator(parent_left_operator_ptr,
                                parent_right_operator_ptr),
            left_window_(left_window),
            right_window_(right_window),
            // init synopsises
            left_synopsis_(left_window_),
            right_synopsis_(right_window_),
            // set attribute comparator
            attribute_comparator_(attribute_comparator) {
            // build new schema and index
            joined_schema_ptr_ = build_joined_schema_();
            set_output_stream(Stream::from_schema(joined_schema_ptr_));
            // set callbacks
            Synopsis::callback_t on_accept = std::tr1::bind(&OperatorJoin::join_synopsis_, this);
            left_synopsis_.set_on_accept(on_accept);
            right_synopsis_.set_on_accept(on_accept);
        }

        typedef std::list<Tuple::ptr_t> join_results_t;


        Tuple::ptr_t process_left_input(Tuple::ptr_t input) {
            left_synopsis_.enqueue(input);
            return Tuple::ptr_t();
        }

        Tuple::ptr_t process_right_input(Tuple::ptr_t input) {
            right_synopsis_.enqueue(input);
            return Tuple::ptr_t();
        }

    private:
        Operator::ptr_t parent_left_operator_ptr_;
        Operator::ptr_t parent_right_operator_ptr_;

        Window left_window_;
        Window right_window_;

        Synopsis left_synopsis_;
        Synopsis right_synopsis_;

        ConditionAttributeComparator::ptr_t attribute_comparator_;

        Schema::ptr_t joined_schema_ptr_;

        Schema::ptr_t build_joined_schema_() {
            return concat_schemas(
                parent_left_operator_ptr_->get_output_schema_ptr(),
                parent_right_operator_ptr_->get_output_schema_ptr()
            );
        }

        inline void
        join_synopsis_() {
            Synopsis::const_iterator left_iter = left_synopsis_.begin();
            Synopsis::const_iterator right_iter = right_synopsis_.begin();

            Synopsis::const_iterator left_iter_end = left_synopsis_.end();
            Synopsis::const_iterator right_iter_end = right_synopsis_.end();

#if 0
            std::cout << "Left Synopsis\n" << left_synopsis_.toString() << std::endl;
            std::cout << "Right Synopsis\n" << right_synopsis_.toString() << std::endl;
            std::cout.flush();
#endif

            // For now, just conduct a nested-loop join
            for (; left_iter != left_iter_end; ++left_iter) {
                for (; right_iter != right_iter_end; ++right_iter) {
                    if (attribute_comparator_->check(*left_iter, *right_iter)) {
                        output_tuple(Tuple::create(joined_schema_ptr_,
                                                   (*left_iter)->get_concatenated_data(*right_iter)));
                    }
                }
            }
        }

    public:
        virtual std::string toString() {
            return "Join";
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_JOIN_H_ */
