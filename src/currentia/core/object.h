// -*- c++ -*-

#ifndef CURRENTIA_OBJECT_H__
#define CURRENTIA_OBJECT_H__

#include <sstream>
#include <string>
#include <tr1/memory>

namespace currentia {
    enum Type {
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_STRING,
        TYPE_BLOB,
        //
        NUMBER_OF_TYPES
    };

    // util (TODO: this is not appropriate place for this function)
    std::string type2string(enum Type type) {
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

        enum Type type;         // TODO: make it const (immutable)

    private:
        // TODO: union is preferred, but smart pointers are not allowed in `union`
        // (such objects who have constructor and destructor in union are supproted from C++11)
        struct Holder {
            int          int_number;
            double       float_number;
            string_ptr_t string_ptr;
            blob_ptr_t   blob_ptr;
        } holder_;

    public:
        Object(int int_number): type(TYPE_INT) {
            holder_.int_number = int_number;
        }

        Object(double float_number): type(TYPE_FLOAT) {
            holder_.int_number = float_number;
        }

        Object(string_ptr_t string_ptr): type(TYPE_STRING) {
            holder_.string_ptr = string_ptr;
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
                ss << "STRING(" << *holder_.string_ptr  << ")";
                break;
            case TYPE_BLOB:
                ss << "BLOB(" << holder_.blob_ptr << ")";
                break;
            default:
                ss << "UNKNOWN()";
                break;
            }

            return ss.str();
        }

        // immutable
    };
}

#endif  /* ! CURRENTIA_OBJECT_H__ */
