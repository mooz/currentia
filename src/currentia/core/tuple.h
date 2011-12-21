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

        inline
        Object get_value_by_index(int attribute_index) {
            return data_[attribute_index];
        }

        inline
        Object get_value_by_attribute_name(std::string& attribute_name) {
            int attribute_index = schema_ptr_->get_attribute_index_by_name(attribute_name);
            return get_value_by_index(attribute_index);
        }

        friend data_t concat_data(Tuple::ptr_t tuple1, Tuple::ptr_t tuple2) {
            data_t new_data;

            new_data.insert(new_data.end(), tuple1->data_.begin(), tuple1->data_.end());
            new_data.insert(new_data.end(), tuple2->data_.begin(), tuple2->data_.end());

            return new_data;
        }

    private:
        data_t data_;
        Schema::ptr_t schema_ptr_;

        Tuple(Schema::ptr_t schema_ptr, data_t data) {
            schema_ptr_ = schema_ptr;
            data_ = data;
        }
    };
}

#endif  /* ! CURRENTIA_TUPLE_H__ */
