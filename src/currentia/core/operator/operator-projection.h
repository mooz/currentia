// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_PROJECTION_H_
#define CURRENTIA_OPERATOR_PROJECTION_H_

#include "currentia/core/object.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"

namespace currentia {
    class OperatorProjection: public SingleInputOperator {
    public:
        typedef std::list<std::string> target_attribute_names_t;
        typedef std::vector<int> target_attribute_indices_t;

        OperatorProjection(Operator::ptr_t parent_operator_ptr,
                           target_attribute_names_t target_attribute_names):
            SingleInputOperator(parent_operator_ptr),
            // Initialize members
            target_attribute_names_(target_attribute_names) {
            old_schema_ptr_ = parent_operator_ptr_->get_output_schema_ptr();
            build_new_schema_and_indices();
            // Arrange an output stream
            set_output_stream(Stream::from_schema(new_schema_ptr_));
        }

        void process_single_input(Tuple::ptr_t input_tuple) {
            output_tuple(project_attributes(input_tuple));
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
                if (target_attribute_index_in_original_schema < 0) {
                    std::stringstream ss;
                    ss << "Requested attribute \"" << target_attribute_name << "\" is not found in specified schema: " << old_schema_ptr_->toString();
                    throw ss.str();
                }
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

            auto projected_tuple = Tuple::create(new_schema_ptr_,
                                                 data,
                                                 target_tuple_ptr->get_arrived_time());
#ifdef CURRENTIA_ENABLE_TRANSACTION
            projected_tuple->set_lwm(target_tuple_ptr->get_lwm());
#endif
            return projected_tuple;
        }

    public:
        std::string toString() const {
            std::stringstream ss;
            ss << parent_operator_ptr_->toString() << "\n -> " << get_name() << "(";

            target_attribute_names_t::const_iterator iter = target_attribute_names_.begin();
            while (iter != target_attribute_names_.end()) {
                ss << *iter;
                iter++;
                if (iter != target_attribute_names_.end())
                    ss << ", ";
            }

            ss << ")";
            return ss.str();
        }

        std::string get_name() const {
            return std::string("Projection");
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_PROJECTION_H_ */
