// -*- c++ -*-

#ifndef CURRENTIA_OPERATIONS_H_
#define CURRENTIA_OPERATIONS_H_

#include "currentia/core/object.h"
#include <sstream>

namespace currentia {
    namespace operations {
        enum OPERATION_ERROR {
            TYPE_MISMATCH,
            UNSUPPORTED_OPERATION
        };

        Object operation_add(const Object& left,
                             const Object& right) {
            std::stringstream ss;

            switch (left.type) {
            case Object::INT:
                switch (right.type) {
                case Object::INT:
                    return Object(left.holder_.int_number + right.holder_.int_number);
                case Object::FLOAT:
                    return Object(static_cast<double>(left.holder_.int_number) + right.holder_.float_number);
                case Object::STRING:
                    ss << left.holder_.int_number;
                    return Object(*Object::string_ptr_t(new std::string(ss.str())) + *right.holder_.string_ptr);
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::FLOAT:
                switch (right.type) {
                case Object::INT:
                    return Object(left.holder_.float_number + static_cast<double>(right.holder_.int_number));
                case Object::FLOAT:
                    return Object(left.holder_.float_number + right.holder_.float_number);
                case Object::STRING:
                    ss << left.holder_.float_number;
                    return Object(*Object::string_ptr_t(new std::string(ss.str())) + *right.holder_.string_ptr);
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::STRING:
                switch (right.type) {
                case Object::INT:
                    ss << right.holder_.int_number;
                    return Object(*left.holder_.string_ptr + *Object::string_ptr_t(new std::string(ss.str())));
                case Object::FLOAT:
                    ss << right.holder_.float_number;
                    return Object(*left.holder_.string_ptr + *Object::string_ptr_t(new std::string(ss.str())));
                case Object::STRING:
                    return Object(*left.holder_.string_ptr + *right.holder_.string_ptr);
                default:
                    throw TYPE_MISMATCH;
                }
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        Object operation_subtract(const Object& left,
                                         const Object& right) {
            switch (left.type) {
            case Object::INT:
                switch (right.type) {
                case Object::INT:
                    return Object(left.holder_.int_number - right.holder_.int_number);
                case Object::FLOAT:
                    return Object(static_cast<double>(left.holder_.int_number) - right.holder_.float_number);
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::FLOAT:
                switch (right.type) {
                case Object::INT:
                    return Object(left.holder_.float_number - static_cast<double>(right.holder_.int_number));
                case Object::FLOAT:
                    return Object(left.holder_.float_number - right.holder_.float_number);
                default:
                    throw TYPE_MISMATCH;
                }
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        Object operation_multiply(const Object& left,
                                         const Object& right) {
            switch (left.type) {
            case Object::INT:
                switch (right.type) {
                case Object::INT:
                    return Object(left.holder_.int_number * right.holder_.int_number);
                case Object::FLOAT:
                    return Object(static_cast<double>(left.holder_.int_number) * right.holder_.float_number);
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::FLOAT:
                switch (right.type) {
                case Object::INT:
                    return Object(left.holder_.float_number * static_cast<double>(right.holder_.int_number));
                case Object::FLOAT:
                    return Object(left.holder_.float_number * right.holder_.float_number);
                default:
                    throw TYPE_MISMATCH;
                }
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        Object operation_divide(const Object& left,
                                       const Object& right) {
            switch (left.type) {
            case Object::INT:
                switch (right.type) {
                case Object::INT:
                    return Object(left.holder_.int_number / right.holder_.int_number);
                case Object::FLOAT:
                    return Object(static_cast<double>(left.holder_.int_number) / right.holder_.float_number);
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::FLOAT:
                switch (right.type) {
                case Object::INT:
                    return Object(left.holder_.float_number / static_cast<double>(right.holder_.int_number));
                case Object::FLOAT:
                    return Object(left.holder_.float_number / right.holder_.float_number);
                default:
                    throw TYPE_MISMATCH;
                }
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }
    };
}

#endif  /* ! CURRENTIA_OPERATIONS_H_ */
