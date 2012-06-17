// -*- c++ -*-

#ifndef CURRENTIA_OPERATIONS_H_
#define CURRENTIA_OPERATIONS_H_

#include "currentia/core/object.h"

namespace currentia {
    namespace operations {
        enum OPERATION_ERROR {
            TYPE_MISMATCH,
            UNSUPPORTED_OPERATION
        };

        Object::ptr_t operation_add(const Object& left,
                                    const Object& right) {
            if (left.type != right.type)
                throw TYPE_MISMATCH;

            switch (left.type) {
            case Object::INT:
                return Object::ptr_t(new Object(left.holder_.int_number + right.holder_.int_number));
            case Object::FLOAT:
                return Object::ptr_t(new Object(left.holder_.float_number + right.holder_.float_number));
            case Object::STRING:
                return Object::ptr_t(new Object(*left.holder_.string_ptr + *right.holder_.string_ptr));
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        Object::ptr_t operation_subtract(const Object& left,
                                         const Object& right) {
            if (left.type != right.type)
                throw TYPE_MISMATCH;

            switch (left.type) {
            case Object::INT:
                return Object::ptr_t(new Object(left.holder_.int_number - right.holder_.int_number));
            case Object::FLOAT:
                return Object::ptr_t(new Object(left.holder_.float_number - right.holder_.float_number));
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        Object::ptr_t operation_multiply(const Object& left,
                                         const Object& right) {
            if (left.type != right.type)
                throw TYPE_MISMATCH;

            switch (left.type) {
            case Object::INT:
                return Object::ptr_t(new Object(left.holder_.int_number * right.holder_.int_number));
            case Object::FLOAT:
                return Object::ptr_t(new Object(left.holder_.float_number * right.holder_.float_number));
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        Object::ptr_t operation_divide(const Object& left,
                                       const Object& right) {
            if (left.type != right.type)
                throw TYPE_MISMATCH;

            switch (left.type) {
            case Object::INT:
                return Object::ptr_t(new Object(left.holder_.int_number / right.holder_.int_number));
            case Object::FLOAT:
                return Object::ptr_t(new Object(left.holder_.float_number / right.holder_.float_number));
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }
    };
}

#endif  /* ! CURRENTIA_OPERATIONS_H_ */
