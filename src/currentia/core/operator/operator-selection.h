// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_SELECTION_H__
#define CURRENTIA_OPERATOR_SELECTION_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/comparator.h"

namespace currentia {
    class OperatorSelection: public Operator {
        // TODO: support condition from multiple comparision (AND, OR, BETWEEN, ...)
        enum ComparatorType comparator_;
        std::string target_attribute_name_;
        Object comparator_value_;

    public:
        // created from visitor pattern?
        // TODO: think about the way of construction from declarative queries
        OperatorSelection(Operator::ptr_t parent_operator_ptr,
                          enum ComparatorType comparator,
                          std::string target_attribute_name,
                          Object comparator_value):
            comparator_(comparator),
            target_attribute_name_(target_attribute_name),
            comparator_value_(comparator_value) {
            parent_operator_ptr_ = parent_operator_ptr;
            output_schema_ptr_ = parent_operator_ptr_->output_schema_ptr_;
        }

        Tuple::ptr_t next() {
            int target_attribute_index = -1;

            while (Tuple::ptr_t target_tuple_ptr = parent_operator_ptr_->next()) {
                // we assume branch prediction of the processor pass this comparation
                if (target_attribute_index == -1) {
                    // for fast access to the attributes
                    target_attribute_index = target_tuple_ptr->
                                             schema_ptr_->
                                             attributes_index_[target_attribute_name_];
                }

                // always compared by [target_value <comparator_> comparator_value_]
                Object target_value = target_tuple_ptr->data_[target_attribute_index];

                if (target_value.compare(comparator_value_, comparator_)) {
                    // return tuple which fulfills condition
                    return target_tuple_ptr;
                }
                // otherwise, tuple will be *deleted* by the shared_ptr
            }

            return Tuple::ptr_t(); // NULL
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_SELECTION_H__ */
