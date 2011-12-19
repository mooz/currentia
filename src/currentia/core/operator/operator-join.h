// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_JOIN_H__
#define CURRENTIA_OPERATOR_JOIN_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/comparator.h"

namespace currentia {
    class OperatorJoin: public Operator {
        // TODO: support condition from multiple comparision (AND, OR)
        enum ComparatorType comparator_;
        std::string target_attribute_name_;

        Operator::ptr_t parent_left_operator_ptr_;
        Operator::ptr_t parent_right_operator_ptr_;
        long ignore_right_column_index;

        Schema::ptr_t joined_schema_ptr_;

        int window_width_;
        int slide_width_;

        void build_new_schema() {
            joined_schema_ptr_ = Schema::ptr_t(new Schema());

            Schema::attributes_t left_attributes = parent_left_operator_ptr_->
                                                   output_schema_ptr_->
                                                   attributes_;
            Schema::attributes_t right_attributes = parent_right_operator_ptr_->
                                                    output_schema_ptr_->
                                                    attributes_;

            Schema::attributes_t::iterator left_iter = left_attributes.begin();
            for (; left_iter != left_attributes.end(); left_iter++) {
                joined_schema_ptr_->add_attribute(*left_iter);
            }

            ignore_right_column_index = -1;
            int loop_count = 0;
            Schema::attributes_t::iterator right_iter = right_attributes.begin();
            for (; right_iter != right_attributes.end(); right_iter++) {
                if ((*right_iter).name != target_attribute_name_) {
                    // if it is not the target attribute
                    joined_schema_ptr_->add_attribute(*right_iter);
                } else {
                    ignore_right_column_index = loop_count;
                }
                loop_count++;
            }

            joined_schema_ptr_->freeze();
        }

    public:
        OperatorJoin(Operator::ptr_t parent_left_operator_ptr,
                     Operator::ptr_t parent_right_operator_ptr,
                     enum ComparatorType comparator,
                     std::string target_attribute_name,
                     int window_width,
                     int slide_width):
            comparator_(comparator),
            target_attribute_name_(target_attribute_name),
            window_width_(window_width) {
            // init
            parent_left_operator_ptr_ = parent_left_operator_ptr;
            parent_right_operator_ptr_ = parent_right_operator_ptr;
            // build new schema and index
            build_new_schema();
            output_schema_ptr_ = joined_schema_ptr_;
        }

        Tuple::ptr_t next() {
            while (true) {
                Tuple::ptr_t left_tuple_ptr = parent_left_operator_ptr_->next();
                Tuple::ptr_t right_tuple_ptr = parent_right_operator_ptr_->next();

                Tuple::data_t left_data = left_tuple_ptr->data_;
                Tuple::data_t right_data = right_tuple_ptr->data_;

                Tuple::data_t joined_data;

                Tuple::data_t::iterator left_data_iter = left_data.begin();
                for (; left_data_iter != left_data.end(); ++left_data_iter) {
                    joined_data.push_back(*left_data_iter);
                }

                int loop_count = 0;
                Tuple::data_t::iterator right_data_iter = right_data.begin();
                for (; right_data_iter != right_data.end(); ++right_data_iter) {
                    if (loop_count != ignore_right_column_index) {
                        joined_data.push_back(*right_data_iter);
                    }
                    loop_count++;
                }

                return Tuple::create(joined_schema_ptr_, joined_data);
            }

            return Tuple::ptr_t(); // NULL
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_JOIN_H__ */
