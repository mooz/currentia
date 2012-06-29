// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_MEAN_H_
#define CURRENTIA_OPERATOR_MEAN_H_

#include "currentia/core/attribute.h"
#include "currentia/core/object.h"
#include "currentia/core/operation/operations.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/trait-aggregation-operator.h"
#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"

#include <functional>

namespace currentia {
    class OperatorMean: public SingleInputOperator,
                        public TraitAggregationOperator {
        std::string target_attribute_name_;
        Object sum_;
        Object window_width_object_;

    public:
        OperatorMean(Operator::ptr_t parent_operator_ptr,
                     Window window,
                     const std::string& target_attribute_name):
            SingleInputOperator(parent_operator_ptr),
            TraitAggregationOperator(window,
                                     std::bind(&OperatorMean::calculate_mean_, this)),
            target_attribute_name_(target_attribute_name),
            sum_(0.0),
            window_width_object_(static_cast<double>(window.width)) {
            // Setup schema
            Schema::ptr_t output_stream_schema(new Schema());
            output_stream_schema->add_attribute(target_attribute_name, Object::FLOAT);
            set_output_stream(Stream::from_schema(output_stream_schema));
        }

        void process_single_input(Tuple::ptr_t input_tuple) {
            synopsis_.enqueue(input_tuple);
        }

    private:
        void calculate_mean_() {
            sum_ = Object(0.0);
            Synopsis::const_iterator iter = synopsis_.begin();
            Synopsis::const_iterator iter_end = synopsis_.end();

            for (; iter != iter_end; ++iter) {
                sum_ = operations::operation_add(
                    sum_,
                    (*iter)->get_value_by_attribute_name(target_attribute_name_) // TODO: build index
                );
            }

            Tuple::ptr_t mean_tuple = Tuple::create_easy(
                get_output_schema_ptr(),
                operations::operation_divide(sum_, window_width_object_)
            );
            output_tuple(mean_tuple);
        }

    public:
        std::string toString() const {
            std::stringstream ss;
            ss << parent_operator_ptr_->toString() << "\n -> " << get_name() << "(" << window_.toString() << ")";
            return ss.str();
        }

        std::string get_name() const {
            return std::string("Mean");
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_MEAN_H_ */
