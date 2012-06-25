// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_JOIN_H_
#define CURRENTIA_OPERATOR_JOIN_H_

#include "currentia/core/attribute.h"
#include "currentia/core/object.h"
#include "currentia/core/operation/operations.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/synopsis.h"
#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"
#include "currentia/core/window.h"

#include <tr1/functional>

namespace currentia {
    class OperatorMean: public SingleInputOperator {
        Window window_;
        Synopsis synopsis_;
        Attribute target_attribute_;
        Object sum_;
        Object window_width_object_;

    public:
        OperatorMean(Operator::ptr_t parent_operator_ptr,
                     Window window,
                     Attribute target_attribute):
            SingleInputOperator(parent_operator_ptr_),
            window_(window),
            synopsis_(window),
            target_attribute_(target_attribute),
            sum_(0.0),
            window_width_object_(static_cast<double>(window.width)) {
            Schema::ptr_t output_stream_schema(new Schema());
            output_stream_schema->add_attribute(target_attribute.name, Object::FLOAT);
            set_output_stream(Stream::from_schema(output_stream_schema));
            // Setup handler
            Synopsis::callback_t on_accept = std::tr1::bind(&OperatorMean::calculate_mean_, this);
            synopsis_.set_on_accept(on_accept);
        }

        Tuple::ptr_t process_single_input(Tuple::ptr_t input_tuple) {
            synopsis_.enqueue(input_tuple);
            return Tuple::ptr_t();
        }

    private:
        void calculate_mean_() {
            sum_ = Object(0.0);
            Synopsis::const_iterator iter = synopsis_.begin();
            Synopsis::const_iterator iter_end = synopsis_.end();

            for (; iter != iter_end; ++iter) {
                sum_ = operations::operation_add(
                    sum_,
                    (*iter)->get_value_by_attribute_name(target_attribute_.name) // TODO: build index
                );
            }

            Tuple::data_t data;
            data.push_back(operations::operation_divide(sum_, window_width_object_));
            output_tuple(Tuple::create(get_output_schema_ptr(), data));
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_JOIN_H_ */
