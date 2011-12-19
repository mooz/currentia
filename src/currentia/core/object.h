// -*- c++ -*-

#ifndef CURRENTIA_OBJECT_H__
#define CURRENTIA_OBJECT_H__

#include <sstream>
#include <string>

#include "currentia/core/pointer.h"
#include "currentia/core/operator/comparator.h"

namespace currentia {
    enum ObjectType {
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_STRING,
        TYPE_BLOB,
        //
        NUMBER_OF_TYPES
    };

    // util (TODO: this is not appropriate place for this function)
    std::string type2string(enum ObjectType type) {
        switch (type) {
        case TYPE_INT:
            return std::string("TYPE_INT");
        case TYPE_FLOAT:
            return std::string("TYPE_FLOAT");
        case TYPE_STRING:
            return std::string("TYPE_STRING");
        case TYPE_BLOB:
            return std::string("TYPE_BLOB");
        default:
            return std::string("TYPE_UNKNOWN");
        }
    }

    class Object {
    public:
        typedef std::tr1::shared_ptr<Object> ptr_t;

        // typedef int int_number_t;
        // typedef double double_number_t;
        typedef std::tr1::shared_ptr<std::string> string_ptr_t;
        typedef std::tr1::shared_ptr<char*> blob_ptr_t;

        enum ObjectType type;         // TODO: make it const (immutable)

    private:
        template <typename T>
        inline bool generic_compare(T& x, T& y, enum ComparatorType comparator) {
            switch (comparator) {
            case COMPARATOR_EQUAL:              // ==
                return x == y;
            case COMPARATOR_NOT_EQUAL:          // !=
                return x != y;
            case COMPARATOR_LESS_THAN:          // <
                return x < y;
            case COMPARATOR_LESS_THAN_EQUAL:    // <=
                return x <= y;
            case COMPARATOR_GREATER_THAN:       // >
                return x > y;
            case COMPARATOR_GREATER_THAN_EQUAL: // >=
                return x >= y;
            default:
                return false;
            }
        }

    public:
        // TODO: make it private
        // TODO: union is preferred, but smart pointers are not allowed in `union`
        // (such objects who have constructor and destructor in union are supproted from C++11)
        struct Holder {
            int          int_number;
            double       float_number;
            string_ptr_t string_ptr;
            blob_ptr_t   blob_ptr;
        } holder_;

        Object(int int_number): type(TYPE_INT) {
            holder_.int_number = int_number;
        }

        Object(double float_number): type(TYPE_FLOAT) {
            holder_.int_number = float_number;
        }

        Object(string_ptr_t string_ptr): type(TYPE_STRING) {
            holder_.string_ptr = string_ptr;
        }

        Object(std::string& string): type(TYPE_STRING) {
            holder_.string_ptr = string_ptr_t(new std::string(string));
        }

        Object(const char* raw_string): type(TYPE_STRING) {
            holder_.string_ptr = string_ptr_t(new std::string(raw_string));
        }

        Object(blob_ptr_t blob_ptr): type(TYPE_BLOB) {
            holder_.blob_ptr = blob_ptr;
        }

        std::string toString() const {
            std::stringstream ss;

            switch (type) {
            case TYPE_INT:
                ss << "INT(" << holder_.int_number << ")";
                break;
            case TYPE_FLOAT:
                ss << "FLOAT(" << holder_.float_number << ")";
                break;
            case TYPE_STRING:
                ss << "STRING(\"" << *holder_.string_ptr  << "\")";
                break;
            case TYPE_BLOB:
                ss << "BLOB(<#" << holder_.blob_ptr << ">)";
                break;
            default:
                ss << "UNKNOWN()";
                break;
            }

            return ss.str();
        }

        bool compare(Object& target, enum ComparatorType comparator) {
            bool comparison_result = false;

            switch (type) {
            case TYPE_INT:
                comparison_result = generic_compare(holder_.int_number,
                                                    target.holder_.int_number,
                                                    comparator);
                break;
            case TYPE_FLOAT:
                comparison_result = generic_compare(holder_.float_number,
                                                    target.holder_.float_number,
                                                    comparator);
                break;
            case TYPE_STRING:
                // compare deeply
                comparison_result = generic_compare(*(holder_.string_ptr),
                                                    *(target.holder_.string_ptr),
                                                    comparator);
                break;
            case TYPE_BLOB:
                // TODO: implement BLOB object comparison
                break;
            default:
                break;
            }

            return comparison_result;
        }

        // immutable
    };
}

#endif  /* ! CURRENTIA_OBJECT_H__ */
