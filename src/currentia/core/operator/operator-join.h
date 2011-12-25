// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_JOIN_H__
#define CURRENTIA_OPERATOR_JOIN_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"
#include "currentia/core/window.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/condition.h"

namespace currentia {
    class OperatorJoin: public Operator {
    public:
        struct Synopsis {
            Synopsis(Window &window):
                window_(window),
                index_(0) {
            }

            // read tuples to prepare for next join
            void read_tuples_for_next_join(Operator::ptr_t& target_operator) {
                switch (window_.type) {
                case Window::LOGICAL:
                    return read_tuples_for_next_join_logical(target_operator);
                case Window::PHYSICAL:
                    // TODO: support physical window (is it possible in pull-style processing?)
                    return read_tuples_for_next_join_physical(target_operator);
                }
            }

            inline void read_tuples_for_next_join_logical(Operator::ptr_t& target_operator) {
                long tuples_count = tuples_.size();
                long read_count;

                if (tuples_count != window_.width) {
                    // this is the first time
                    read_count = window_.width;
                    tuples_.resize(window_.width);
                } else {
                    read_count = window_.stride; // read a tuple ${stride of sliding window} times
                }

                for (int i = 0; i < read_count; ++i) {
                    tuples_[get_next_index_()] = target_operator->next();
                }
            }

            inline void read_tuples_for_next_join_physical(Operator::ptr_t& target_operator) {
                // not implemented yet
            }

            typedef std::vector<Tuple::ptr_t> tuples_t;
            typedef std::list<Tuple::ptr_t> join_results_t;

            friend inline void
            join_synopsis(Synopsis& left_synopsis,
                          Synopsis& right_synopsis,
                          Schema::ptr_t& joined_schema_ptr,
                          ConditionAttributeComparator::ptr_t& condition,
                          join_results_t* results) {
                tuples_t left_tuples = left_synopsis.tuples_;
                tuples_t right_tuples = right_synopsis.tuples_;

                tuples_t::iterator left_iter = left_tuples.begin();
                tuples_t::iterator right_iter = right_tuples.begin();

                // For now, just conduct a nested-loop join
                for (; left_iter != left_tuples.end(); ++left_iter) {
                    for (; right_iter != right_tuples.end(); ++right_iter) {
                        if (condition->check(*left_iter, *right_iter)) {
                            results->push_back(Tuple::create(joined_schema_ptr, concat_data(*left_iter, *right_iter)));
                        }
                    }
                }
            }

            std::string toString() {
                std::stringstream ss;
                std::vector<Tuple::ptr_t>::iterator iter = tuples_.begin();

                int i = 0;
                for (; iter < tuples_.end(); ++iter) {
                    ss << "[" << i++ << "] " << (*iter)->toString() << std::endl;
                }

                return ss.str();
            }

        private:
            inline long get_next_index_() {
                return index_++ % window_.width;
            }

            std::vector<Tuple::ptr_t> tuples_;
            Window window_;
            long index_;
        };

        OperatorJoin(Operator::ptr_t parent_left_operator_ptr,
                     Window left_window,
                     Operator::ptr_t parent_right_operator_ptr,
                     Window right_window,
                     ConditionAttributeComparator::ptr_t attribute_comparator):
            left_window_(left_window),
            right_window_(right_window),
            // init synopsises
            left_synopsis_(Synopsis(left_window_)),
            right_synopsis_(Synopsis(right_window_)),
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

        std::list<Tuple::ptr_t> remained_join_results_;
        Tuple::ptr_t next() {
            while (remained_join_results_.size() == 0) {
                left_synopsis_.read_tuples_for_next_join(parent_left_operator_ptr_);
                right_synopsis_.read_tuples_for_next_join(parent_right_operator_ptr_);

                // Now, do JOIN
                join_synopsis(left_synopsis_, right_synopsis_,
                              joined_schema_ptr_,
                              attribute_comparator_,
                              &remained_join_results_);
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
    };
}

#endif  /* ! CURRENTIA_OPERATOR_JOIN_H__ */
