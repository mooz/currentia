// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_JOIN_H__
#define CURRENTIA_OPERATOR_JOIN_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/condition.h"

namespace currentia {
    class OperatorJoin: public Operator {
        ConditionAttributeComparator::ptr_t attribute_comparator_;

        Operator::ptr_t parent_left_operator_ptr_;
        Operator::ptr_t parent_right_operator_ptr_;

        Schema::ptr_t joined_schema_ptr_;

        int window_width_;
        int slide_width_;

        std::vector<Tuple::ptr_t> left_synopsis_;
        int left_synopsis_index_;
        std::vector<Tuple::ptr_t> right_synopsis_;
        int right_synopsis_index_;

        Schema::ptr_t build_joined_schema_() {
            return concat_schemas(
                parent_left_operator_ptr_->get_output_schema_ptr(),
                parent_right_operator_ptr_->get_output_schema_ptr()
            );
        }

        void init_synopsis_() {
            left_synopsis_.resize(window_width_);
            right_synopsis_.resize(window_width_);
            left_synopsis_index_ = 0;
            right_synopsis_index_ = 0;
        }

    public:
        OperatorJoin(Operator::ptr_t parent_left_operator_ptr,
                     Operator::ptr_t parent_right_operator_ptr,
                     ConditionAttributeComparator::ptr_t attribute_comparator,
                     int window_width,
                     int slide_width):
            attribute_comparator_(attribute_comparator),
            window_width_(window_width) {
            // init
            parent_left_operator_ptr_ = parent_left_operator_ptr;
            parent_right_operator_ptr_ = parent_right_operator_ptr;
            // init synopsises
            init_synopsis_();
            // build new schema and index
            joined_schema_ptr_ = build_joined_schema_();
        }

        inline
        Schema::ptr_t get_output_schema_ptr() {
            return joined_schema_ptr_;
        }

        Tuple::ptr_t next() {
            while (true) {
                Tuple::ptr_t left_tuple_ptr = parent_left_operator_ptr_->next();
                Tuple::ptr_t right_tuple_ptr = parent_right_operator_ptr_->next();
                Tuple::data_t joined_data = concat_data(left_tuple_ptr, right_tuple_ptr);

                return Tuple::create(joined_schema_ptr_, joined_data);
            }

            return Tuple::ptr_t(); // NULL
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_JOIN_H__ */
