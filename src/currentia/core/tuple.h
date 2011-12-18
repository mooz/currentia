// -*- c++ -*-

#ifndef CURRENTIA_TUPLE_H__
#define CURRENTIA_TUPLE_H__

#include "currentia/core/schema.h"
#include "currentia/core/object.h"
#include "currentia/core/pointer.h"

#include <sstream>              // string_stream
#include <vector>

namespace currentia {
    class Tuple {
    public:
        typedef std::tr1::shared_ptr<Tuple> ptr_t;
        // typedef std::tr1::shared_ptr<std::vector<Object> > data_ptr_t;
        typedef std::vector<Object> data_t;

        static Tuple::ptr_t create(Schema::ptr_t schema_ptr, data_t data) {
            if (!schema_ptr->validate_data(data)) {
                return Tuple::ptr_t(); // NULL pointer
            }

            return Tuple::ptr_t(new Tuple(schema_ptr, data));
        }

        std::string toString() const {
            std::stringstream ss;

            ss << "Tuple(";
            if (data_.size() > 0) {
                ss << "\n";
                data_t::const_iterator it = data_.begin();
                for (; it != data_.end(); ++it) {
                    ss << "  " << it->toString() << "\n";
                }
            }
            ss << ")";

            return ss.str();
        }

        // inline long get_column_number(std::string attribute_name) {
        //     return schema_ptr_->
        // }

        // TODO: make it private
        Schema::ptr_t schema_ptr_;
        data_t data_;

    private:
        Tuple(Schema::ptr_t schema_ptr, data_t data) {
            schema_ptr_ = schema_ptr;
            data_ = data;
        }
    };
}

#endif  /* ! CURRENTIA_TUPLE_H__ */
