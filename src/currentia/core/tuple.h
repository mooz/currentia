// -*- c++ -*-

#ifndef CURRENTIA_TUPLE_H_
#define CURRENTIA_TUPLE_H_

#include "currentia/core/schema.h"
#include "currentia/core/object.h"
#include "currentia/core/pointer.h"

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

#include <sstream>              // string_stream
#include <vector>
#include <ctime>

namespace currentia {
    // Immutable
    class Tuple: private NonCopyable<Tuple>,
                 public Pointable<Tuple> {
    public:
        // typedef std::shared_ptr<std::vector<Object> > data_ptr_t;
        typedef std::vector<Object> data_t;

        enum Type {
            DATA,
            EOS,    // Punctuation (End of Stream)
        };

        static Tuple::ptr_t create_eos() {
            return Tuple::ptr_t(new Tuple(EOS));
        }

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
            switch (type_) {
            case DATA:
                if (data_.size() > 0) {
                    ss << "\n";
                    for (data_t::const_iterator it = data_.begin();
                         it != data_.end();
                         ++it) {
                        ss << "  " << it->toString() << "\n";
                    }
                }
                break;
            case EOS:
                ss << "EOS";
                break;
            }
            ss << ")";

            return ss.str();
        }

        Object get_value_by_index(int attribute_index) const {
            assert_has_attribute_(attribute_index);
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

        const Schema::ptr_t get_schema() {
            return schema_ptr_;
        }

        Type get_type() {
            return type_;
        }

        bool is_system_message() {
            return type_ != DATA;
        }

        bool is_eos() {
            return type_ == EOS;
        }

    private:
        Type type_;

        Schema::ptr_t schema_ptr_;
        data_t data_;
        time_t arrived_time_; // system timestamp

        Tuple(Schema::ptr_t schema_ptr, data_t data, time_t arrived_time):
            type_(DATA),
            schema_ptr_(schema_ptr),
            data_(data),
            arrived_time_(arrived_time) {
        }

        Tuple(Type type): type_(type) {
        }

        void assert_has_attribute_(int index) const {
            if (index < 0 || static_cast<unsigned int>(index) >= data_.size()) {
                std::cerr << "Requested " << index << " but size is " << data_.size() << std::endl;
                throw "This tuple does not have a requested attribute";
            }
        }

    public:
        // Easy helper (See tools/create_easy_generator.rb)

        template <typename T1>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1) {
            data_t data;
            data.push_back(Object(t1));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2, typename T3>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2, const T3& t3) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            data.push_back(Object(t3));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2, typename T3, typename T4>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            data.push_back(Object(t3));
            data.push_back(Object(t4));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2, typename T3, typename T4, typename T5>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            data.push_back(Object(t3));
            data.push_back(Object(t4));
            data.push_back(Object(t5));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            data.push_back(Object(t3));
            data.push_back(Object(t4));
            data.push_back(Object(t5));
            data.push_back(Object(t6));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            data.push_back(Object(t3));
            data.push_back(Object(t4));
            data.push_back(Object(t5));
            data.push_back(Object(t6));
            data.push_back(Object(t7));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            data.push_back(Object(t3));
            data.push_back(Object(t4));
            data.push_back(Object(t5));
            data.push_back(Object(t6));
            data.push_back(Object(t7));
            data.push_back(Object(t8));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            data.push_back(Object(t3));
            data.push_back(Object(t4));
            data.push_back(Object(t5));
            data.push_back(Object(t6));
            data.push_back(Object(t7));
            data.push_back(Object(t8));
            data.push_back(Object(t9));
            return Tuple::create(schema, data);
        }

        template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10) {
            data_t data;
            data.push_back(Object(t1));
            data.push_back(Object(t2));
            data.push_back(Object(t3));
            data.push_back(Object(t4));
            data.push_back(Object(t5));
            data.push_back(Object(t6));
            data.push_back(Object(t7));
            data.push_back(Object(t8));
            data.push_back(Object(t9));
            data.push_back(Object(t10));
            return Tuple::create(schema, data);
        }
    };
}

#endif  /* ! CURRENTIA_TUPLE_H_ */
