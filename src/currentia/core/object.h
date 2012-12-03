// -*- c++ -*-

#ifndef CURRENTIA_OBJECT_H_
#define CURRENTIA_OBJECT_H_

#include <sstream>
#include <string>
#include <iostream>

#include "currentia/core/pointer.h"
#include "currentia/core/operator/comparator.h"
#include "currentia/trait/show.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class Object : public Pointable<Object>,
                   public Show {
    public:
        friend class Operation;

        typedef std::string* string_ptr_t;
        typedef char* blob_ptr_t;

        enum Type {
            INT,
            FLOAT,
            STRING,
            BLOB,
            //
            UNKNOWN
        };

        union Holder {
            int          int_number;
            double       float_number;
            string_ptr_t string_ptr;
            blob_ptr_t   blob_ptr;
        };

        enum Type get_type() const {
            return type_;
        }

    private:
        enum Type type_;
        Holder holder_;

    private:
        template <typename T>
        bool generic_compare_(const T& x, const T& y, Comparator::Type comparator) const {
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

        void set_int_number_(int int_number) {
            holder_.int_number = int_number;
        }

        void set_float_number_(double float_number) {
            holder_.float_number = float_number;
        }

        void set_string_ptr_(string_ptr_t string_ptr) {
            holder_.string_ptr = string_ptr;
        }

        void set_blob_ptr_(blob_ptr_t blob_ptr) {
            holder_.blob_ptr = blob_ptr;
        }

    public:
        int get_int_number() const {
            return holder_.int_number;
        }

        double get_float_number() const {
            return holder_.float_number;
        }

        string_ptr_t get_string_ptr() const {
            return holder_.string_ptr;
        }

        blob_ptr_t get_blob_ptr() const {
            return holder_.blob_ptr;
        }

        ~Object() {
            // No custom deleter
            switch (get_type()) {
            case INT:
                break;
            case FLOAT:
                break;
            case STRING:
                if (get_string_ptr())
                    delete get_string_ptr();
                break;
            case BLOB:
                if (get_blob_ptr())
                    delete get_blob_ptr();
                break;
            case UNKNOWN:
                break;
            }
        }

        Object(int int_number): type_(INT) {
            set_int_number_(int_number);
        }

        Object(long int_number): type_(INT) {
            set_int_number_(int_number);
        }

        Object(double float_number): type_(FLOAT) {
            set_float_number_(float_number);
        }

        // Users have to free a string they try to wrap with Object in
        // the user side (Object class retains a copy of the given
        // string and do not touch with the original pointer)
        Object(string_ptr_t string_ptr): type_(STRING) {
            set_string_ptr_(string_ptr_t(new std::string(*string_ptr)));
        }

        Object(const std::string& string): type_(STRING) {
            set_string_ptr_(string_ptr_t(new std::string(string)));
        }

        Object(const char* raw_string): type_(STRING) {
            set_string_ptr_(string_ptr_t(new std::string(raw_string)));
        }

        // If you do not want to your object to be replicated,
        // consider using BLOB. BLOB retains the original pointer, and
        // frees up the memory it points when the BLOB Object is
        // destructed (You cannot choose custom deleter for
        // now. Always 'delete' is tried to the pointer).
        Object(blob_ptr_t blob_ptr): type_(BLOB) {
            set_blob_ptr_(blob_ptr);
        }

        std::string toString() const {
            std::stringstream ss;

            switch (get_type()) {
            case INT:
                ss << "INT(" << get_int_number() << ")";
                break;
            case FLOAT:
                ss << "FLOAT(" << get_float_number() << ")";
                break;
            case STRING:
                ss << "STRING(\"" << *get_string_ptr()  << "\")";
                break;
            case BLOB:
                ss << "BLOB(<#" << get_blob_ptr() << ">)";
                break;
            default:
                ss << "UNKNOWN()";
                break;
            }

            return ss.str();
        }

        bool compare(const Object& target, Comparator::Type comparator) const {
            bool comparison_result = false;

            if (get_type() != target.get_type()) {
                std::cerr << "Warning: comparing imcompatible type \""
                          << type_to_string(get_type())
                          << "\" and \""
                          << type_to_string(target.get_type()) << "\" " << std::endl;
                // TODO: implement smart casting
                return false;
            }

            switch (get_type()) {
            case INT:
                comparison_result = generic_compare_(get_int_number(),
                                                     target.get_int_number(),
                                                     comparator);
                break;
            case FLOAT:
                comparison_result = generic_compare_(get_float_number(),
                                                     target.get_float_number(),
                                                     comparator);
                break;
            case STRING:
                // compare deeply
                comparison_result = generic_compare_(*(get_string_ptr()),
                                                     *(get_string_ptr()),
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

        bool operator ==(const Object& target) const {
            return this->compare(target, Comparator::EQUAL);
        }

        bool operator !=(const Object& target) const {
            return this->compare(target, Comparator::NOT_EQUAL);
        }

        bool operator <(const Object& target) const {
            return this->compare(target, Comparator::LESS_THAN);
        }

        bool operator <=(const Object& target) const {
            return this->compare(target, Comparator::LESS_THAN_EQUAL);
        }

        bool operator >(const Object& target) const {
            return this->compare(target, Comparator::GREATER_THAN);
        }

        bool operator >=(const Object& target) const {
            return this->compare(target, Comparator::GREATER_THAN_EQUAL);
        }

        static std::string type_to_string(Object::Type type) {
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

        static Type string_to_type(const std::string& type_name) {
            Object::Type type = UNKNOWN;

            if (type_name == "INT")
                type = INT;
            else if (type_name == "FLOAT")
                type = FLOAT;
            else if (type_name == "STRING")
                type = STRING;
            else if (type_name == "BLOB")
                type = BLOB;

            return type;
        }
    };
}

#endif  /* ! CURRENTIA_OBJECT_H_ */
