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
    public:
        struct Window {
            enum Type {
                PHYSICAL,
                LOGICAL
            };

            Window(long width, long slide, Window::Type type = LOGICAL):
                width(width),
                slide(slide),
                type(type) {
            }

            long width;
            long slide;
            Window::Type type;
        };

        struct Synopsis {
            Synopsis(Window &window):
                window_(window),
                index_(0) {
            }

        private:
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

        Tuple::ptr_t next() {
            while (true) {
                Tuple::ptr_t left_tuple_ptr = parent_left_operator_ptr_->next();
                Tuple::ptr_t right_tuple_ptr = parent_right_operator_ptr_->next();
                Tuple::data_t joined_data = concat_data(left_tuple_ptr, right_tuple_ptr);

                return Tuple::create(joined_schema_ptr_, joined_data);
            }

            return Tuple::ptr_t(); // NULL
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
