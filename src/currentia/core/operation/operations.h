// -*- c++ -*-

#ifndef CURRENTIA_OPERATIONS_H_
#define CURRENTIA_OPERATIONS_H_

#include "currentia/core/object.h"
#include <sstream>

namespace currentia {
    class Operation {
    public:
        enum OPERATION_ERROR {
            TYPE_MISMATCH,
            UNSUPPORTED_OPERATION
        };

        static Object add(const Object& left,
                          const Object& right) {
            std::stringstream ss;

            switch (left.get_type()) {
            case Object::INT:
                switch (right.get_type()) {
                case Object::INT:
                    return Object(left.get_int_number() + right.get_int_number());
                case Object::FLOAT:
                    return Object(static_cast<double>(left.get_int_number()) + right.get_float_number());
                case Object::STRING:
                    ss << left.get_int_number();
                    return Object(*Object::string_ptr_t(new std::string(ss.str())) + *right.get_string_ptr());
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::FLOAT:
                switch (right.get_type()) {
                case Object::INT:
                    return Object(left.get_float_number() + static_cast<double>(right.get_int_number()));
                case Object::FLOAT:
                    return Object(left.get_float_number() + right.get_float_number());
                case Object::STRING:
                    ss << left.get_float_number();
                    return Object(*Object::string_ptr_t(new std::string(ss.str())) + *right.get_string_ptr());
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::STRING:
                switch (right.get_type()) {
                case Object::INT:
                    ss << right.get_int_number();
                    return Object(*left.get_string_ptr() + *Object::string_ptr_t(new std::string(ss.str())));
                case Object::FLOAT:
                    ss << right.get_float_number();
                    return Object(*left.get_string_ptr() + *Object::string_ptr_t(new std::string(ss.str())));
                case Object::STRING:
                    return Object(*left.get_string_ptr() + *right.get_string_ptr());
                default:
                    throw TYPE_MISMATCH;
                }
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        static Object subtract(const Object& left,
                               const Object& right) {
            switch (left.get_type()) {
            case Object::INT:
                switch (right.get_type()) {
                case Object::INT:
                    return Object(left.get_int_number() - right.get_int_number());
                case Object::FLOAT:
                    return Object(static_cast<double>(left.get_int_number()) - right.get_float_number());
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::FLOAT:
                switch (right.get_type()) {
                case Object::INT:
                    return Object(left.get_float_number() - static_cast<double>(right.get_int_number()));
                case Object::FLOAT:
                    return Object(left.get_float_number() - right.get_float_number());
                default:
                    throw TYPE_MISMATCH;
                }
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        static Object multiply(const Object& left,
                               const Object& right) {
            switch (left.get_type()) {
            case Object::INT:
                switch (right.get_type()) {
                case Object::INT:
                    return Object(left.get_int_number() * right.get_int_number());
                case Object::FLOAT:
                    return Object(static_cast<double>(left.get_int_number()) * right.get_float_number());
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::FLOAT:
                switch (right.get_type()) {
                case Object::INT:
                    return Object(left.get_float_number() * static_cast<double>(right.get_int_number()));
                case Object::FLOAT:
                    return Object(left.get_float_number() * right.get_float_number());
                default:
                    throw TYPE_MISMATCH;
                }
            default:
                throw UNSUPPORTED_OPERATION;
            }
        }

        static Object divide(const Object& left,
                             const Object& right) {
            switch (left.get_type()) {
            case Object::INT:
                switch (right.get_type()) {
                case Object::INT:
                    return Object(left.get_int_number() / right.get_int_number());
                case Object::FLOAT:
                    return Object(static_cast<double>(left.get_int_number()) / right.get_float_number());
                default:
                    throw TYPE_MISMATCH;
                }
            case Object::FLOAT:
                switch (right.get_type()) {
                case Object::INT:
                    return Object(left.get_float_number() / static_cast<double>(right.get_int_number()));
                case Object::FLOAT:
                    return Object(left.get_float_number() / right.get_float_number());
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
