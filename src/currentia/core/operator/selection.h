// -*- c++ -*-

#ifndef CURRENTIA_SELECTION_H__
#define CURRENTIA_SELECTION_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/comparator.h"

namespace currentia {
    class SelectionOperator: public Operator {
        // TODO: support condition from multiple comparision (AND, OR, BETWEEN, ...)
        enum ComparatorType comparator_;
        Object comparator_value_;

    public:
        // created from visitor pattern?
        // TODO: think about the way of construction from declarative queries
        SelectionOperator(Stream::ptr_t input_stream,
                          enum ComparatorType comparator,
                          Object comparator_value):
            Operator(input_stream), // constructor of the super class
            comparator_(comparator),
            comparator_value_(comparator_value) {
        }

        Tuple::ptr_t next() {
            std::string target_column_name = "";
            int target_column_index = -1;

            while (Tuple::ptr_t target_tuple_ptr = input_stream_->dequeue()) {
                // we assume branch prediction of the processor pass this comparation
                if (target_column_index == -1) {
                    // for fast access to the attributes
                    target_column_index = target_tuple_ptr->
                                          schema_ptr_->
                                          attributes_index_[target_column_name];
                }

                // always compared by [target_value <comparator_> comparator_value_]
                Object target_value = target_tuple_ptr->data_[target_column_index];

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

#endif  /* ! CURRENTIA_SELECTION_H__ */
