// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_PROJECTION_H_
#define CURRENTIA_OPERATOR_PROJECTION_H_

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"

namespace currentia {
    class OperatorProjection: public Operator {
        Operator::ptr_t parent_operator_ptr_;

    public:
        typedef std::list<std::string> target_attribute_names_t;
        typedef std::vector<int> target_attribute_indices_t;

        OperatorProjection(Operator::ptr_t parent_operator_ptr,
                           target_attribute_names_t target_attribute_names):
            target_attribute_names_(target_attribute_names) {
            parent_operator_ptr_ = parent_operator_ptr;
            old_schema_ptr_ = parent_operator_ptr_->get_output_schema_ptr();
            build_new_schema_and_indices();
        }

        inline
        Schema::ptr_t get_output_schema_ptr() {
            return new_schema_ptr_;
        }

        inline
        Tuple::ptr_t next_implementation() {
            while (Tuple::ptr_t target_tuple_ptr = parent_operator_ptr_->next()) {
                if (target_tuple_ptr->is_system_message())
                    return target_tuple_ptr;
                return project_attributes(target_tuple_ptr);
            }
            return Tuple::ptr_t(); // NULL
        }

    private:
        target_attribute_names_t target_attribute_names_;
        target_attribute_indices_t target_attribute_indices_;

        Schema::ptr_t old_schema_ptr_;
        Schema::ptr_t new_schema_ptr_;

        void build_new_schema_and_indices() {
            new_schema_ptr_ = Schema::ptr_t(new Schema());

            target_attribute_names_t::const_iterator iter = target_attribute_names_.begin();

            for (; iter != target_attribute_names_.end(); ++iter) {
                std::string target_attribute_name = *iter;

                int target_attribute_index_in_original_schema =
                    old_schema_ptr_->get_attribute_index_by_name(target_attribute_name);
                target_attribute_indices_.push_back(target_attribute_index_in_original_schema);

                Attribute original_attribute =
                    old_schema_ptr_->get_attribute_by_index(target_attribute_index_in_original_schema);

                new_schema_ptr_->add_attribute(original_attribute.name, original_attribute.type);
            }

            new_schema_ptr_->freeze();
        }

        Tuple::ptr_t project_attributes(Tuple::ptr_t target_tuple_ptr) const {
            Tuple::data_t data;

            target_attribute_indices_t::const_iterator iter = target_attribute_indices_.begin();

            for (; iter != target_attribute_indices_.end(); ++iter) {
                int attribute_index = *iter;
                data.push_back(target_tuple_ptr->get_value_by_index(attribute_index));
            }

            return Tuple::create(new_schema_ptr_, data);
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_PROJECTION_H_ */
