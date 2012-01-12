// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_JOIN_H_
#define CURRENTIA_OPERATOR_JOIN_H_

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"
#include "currentia/core/window.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/synopsis.h"

namespace currentia {
    class OperatorJoin: public Operator {
    public:
        OperatorJoin(Operator::ptr_t parent_left_operator_ptr,
                     Window left_window,
                     Operator::ptr_t parent_right_operator_ptr,
                     Window right_window,
                     ConditionAttributeComparator::ptr_t attribute_comparator):
            left_window_(left_window),
            right_window_(right_window),
            // init synopsises
            left_synopsis_(left_window_),
            right_synopsis_(right_window_),
            // set attribute comparator
            attribute_comparator_(attribute_comparator) {
            // save pointers
            parent_left_operator_ptr_ = parent_left_operator_ptr;
            parent_right_operator_ptr_ = parent_right_operator_ptr;
            // build new schema and index
            joined_schema_ptr_ = build_joined_schema_();
        }

        inline
        Schema::ptr_t get_output_schema_ptr() {
            return joined_schema_ptr_;
        }

        typedef std::list<Tuple::ptr_t> join_results_t;

        std::list<Tuple::ptr_t> remained_join_results_;
        Tuple::ptr_t next() {
            while (remained_join_results_.empty()) {
                left_synopsis_.read_next_tuples(parent_left_operator_ptr_);
                right_synopsis_.read_next_tuples(parent_right_operator_ptr_);

                // Now, do JOIN
                join_synopsis_();
            }

            // return a tuple from remained result
            Tuple::ptr_t next_tuple = remained_join_results_.front();
            remained_join_results_.pop_front();
            return next_tuple;
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
                        remained_join_results_.push_back(
                            Tuple::create(joined_schema_ptr_,
                                          (*left_iter)->get_concatenated_data(*right_iter))
                        );
                    }
                }
            }
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_JOIN_H_ */
