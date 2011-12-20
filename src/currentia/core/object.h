// -*- c++ -*-

#ifndef CURRENTIA_OBJECT_H__
#define CURRENTIA_OBJECT_H__

#include <sstream>
#include <string>
#include <iostream>

#include "currentia/core/pointer.h"
#include "currentia/core/operator/comparator.h"

namespace currentia {
    class Object {
    public:
        enum Type {
            INT,
            FLOAT,
            STRING,
            BLOB,
            //
            NUMBER_OF_TYPES
        };

        typedef std::tr1::shared_ptr<Object> ptr_t;

        // typedef int int_number_t;
        // typedef double double_number_t;
        typedef std::tr1::shared_ptr<std::string> string_ptr_t;
        typedef std::tr1::shared_ptr<char*> blob_ptr_t;

        enum Type type;         // TODO: make it const (immutable)

    private:
        template <typename T>
        inline bool generic_compare(T& x, T& y, Comparator::Type comparator) {
            switch (comparator) {
            case Comparator::EQUAL:              // ==
                return x == y;
            case Comparator::NOT_EQUAL:          // !=
                return x != y;
            case Comparator::LESS_THAN:          // <
                return x < y;
            case Comparator::LESS_THAN_EQUAL:    // <=
                return x <= y;
            case Comparator::GREATER_THAN:       // >
                return x > y;
            case Comparator::GREATER_THAN_EQUAL: // >=
                return x >= y;
            default:
                return false;
            }
        }

        // TODO: union is preferred, but smart pointers are not allowed in `union`
        // (such objects who have constructor and destructor in union are supproted from C++11)
        struct Holder {
            int          int_number;
            double       float_number;
            string_ptr_t string_ptr;
            blob_ptr_t   blob_ptr;
        } holder_;

    public:
        Object(int int_number): type(INT) {
            holder_.int_number = int_number;
        }

        Object(double float_number): type(FLOAT) {
            holder_.float_number = float_number;
        }

        Object(string_ptr_t string_ptr): type(STRING) {
            holder_.string_ptr = string_ptr;
        }

        Object(std::string& string): type(STRING) {
            holder_.string_ptr = string_ptr_t(new std::string(string));
        }

        Object(const char* raw_string): type(STRING) {
            holder_.string_ptr = string_ptr_t(new std::string(raw_string));
        }

        Object(blob_ptr_t blob_ptr): type(BLOB) {
            holder_.blob_ptr = blob_ptr;
        }

        std::string toString() const {
            std::stringstream ss;

            switch (type) {
            case INT:
                ss << "INT(" << holder_.int_number << ")";
                break;
            case FLOAT:
                ss << "FLOAT(" << holder_.float_number << ")";
                break;
            case STRING:
                ss << "STRING(\"" << *holder_.string_ptr  << "\")";
                break;
            case BLOB:
                ss << "BLOB(<#" << holder_.blob_ptr << ">)";
                break;
            default:
                ss << "UNKNOWN()";
                break;
            }

            return ss.str();
        }

        bool compare(Object& target, Comparator::Type comparator) {
            bool comparison_result = false;

            if (type != target.type) {
                std::cerr << "Warning: comparing imcompatible type \""
                          << type2string(type)
                          << "\" and \""
                          << type2string(target.type) << "\" " << std::endl;
                // TODO: implement smart casting
                return false;
            }

            switch (type) {
            case INT:
                comparison_result = generic_compare(holder_.int_number,
                                                    target.holder_.int_number,
                                                    comparator);
                break;
            case FLOAT:
                comparison_result = generic_compare(holder_.float_number,
                                                    target.holder_.float_number,
                                                    comparator);
                break;
            case STRING:
                // compare deeply
                comparison_result = generic_compare(*(holder_.string_ptr),
                                                    *(target.holder_.string_ptr),
                                                    comparator);
                break;
            case BLOB:
                // TODO: implement BLOB object comparison
                break;
            default:
                break;
            }

            return comparison_result;
        }

        static std::string type2string(Object::Type type) {
            switch (type) {
            case INT:
                return std::string("TYPE_INT");
            case FLOAT:
                return std::string("TYPE_FLOAT");
            case STRING:
                return std::string("TYPE_STRING");
            case BLOB:
                return std::string("TYPE_BLOB");
            default:
                return std::string("TYPE_UNKNOWN");
            }
        }
    };
}

#endif  /* ! CURRENTIA_OBJECT_H__ */
