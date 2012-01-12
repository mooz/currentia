// -*- c++ -*-

#ifndef CURRENTIA_TUPLE_H_
#define CURRENTIA_TUPLE_H_

#include "currentia/core/schema.h"
#include "currentia/core/object.h"
#include "currentia/core/pointer.h"
#include "currentia/trait/non-copyable.h"

#include <sstream>              // string_stream
#include <vector>
#include <ctime>

namespace currentia {
    class Tuple: private NonCopyable<Tuple> {
    public:
        typedef std::tr1::shared_ptr<Tuple> ptr_t;
        // typedef std::tr1::shared_ptr<std::vector<Object> > data_ptr_t;
        typedef std::vector<Object> data_t;

        static Tuple::ptr_t create(Schema::ptr_t schema_ptr,
                                   data_t data) {
            return Tuple::create(schema_ptr, data, time(NULL));
        }

        static Tuple::ptr_t create(Schema::ptr_t schema_ptr,
                                   data_t data,
                                   time_t arrived_time) {
            if (!schema_ptr->validate_data(data)) {
                return Tuple::ptr_t(); // NULL pointer
            }

            return Tuple::ptr_t(new Tuple(schema_ptr, data, arrived_time));
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

        Object get_value_by_index(int attribute_index) const {
            return data_[attribute_index];
        }

        Object get_value_by_attribute_name(const std::string& attribute_name) const {
            int attribute_index = schema_ptr_->get_attribute_index_by_name(attribute_name);
            return get_value_by_index(attribute_index);
        }

        data_t get_concatenated_data(const Tuple::ptr_t& target) {
            data_t new_data;

            new_data.insert(new_data.end(), data_.begin(), data_.end());
            new_data.insert(new_data.end(), target->data_.begin(), target->data_.end());

            return new_data;
        }

        inline
        time_t get_arrived_time() const {
            return arrived_time_;
        }

    private:
        Schema::ptr_t schema_ptr_;
        data_t data_;
        time_t arrived_time_; // system timestamp

        Tuple(Schema::ptr_t schema_ptr, data_t data, time_t arrived_time):
            schema_ptr_(schema_ptr),
            data_(data),
            arrived_time_(arrived_time) {
        }
    };
}

#endif  /* ! CURRENTIA_TUPLE_H_ */
