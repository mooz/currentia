// -*- c++ -*-

#ifndef CURRENTIA_TUPLE_H_
#define CURRENTIA_TUPLE_H_

#include "currentia/core/object.h"
#include "currentia/core/pointer.h"
#include "currentia/core/schema.h"
#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"
#include "currentia/trait/show.h"

#include <sstream>              // string_stream
#include <vector>
#include <ctime>

#ifdef CURRENTIA_ENABLE_TRANSACTION
#include <unordered_map>
#endif

namespace currentia {
#ifdef CURRENTIA_ENABLE_TRANSACTION
        // (Since relation.h refers tuple.h, we cannot include
        // relation.h and use Relation::ptr)
        // TODO: more flexible (Do not hard-code 'Relation', allow user to select 'Tuple' or 'Page')
    class Relation;
#endif

    // Immutable
    class Tuple: private NonCopyable<Tuple>,
                 public Pointable<Tuple>,
                 public Show {
    public:
        typedef std::vector<Object> data_t;

        enum Type {
            DATA,
            EOS,    // Punctuation (End of Stream)
        };

    private:
        Type type_;

        Schema::ptr_t schema_ptr_;
        data_t data_;
        time_t arrived_time_; // system timestamp

#ifdef CURRENTIA_ENABLE_TRANSACTION
        time_t hwm_; // high water mark
        typedef std::map<std::shared_ptr<Relation>, long> version_numbers_t;
        version_numbers_t referenced_version_numbers_;

    public:
        void set_hwm(time_t hwm) {
            hwm_ = hwm;
        }

        time_t get_hwm() const {
            return hwm_;
        }

        void set_referenced_version_number(const std::shared_ptr<Relation>& relation, long verison) {
            referenced_version_numbers_[relation] = verison;
        }

        long get_referenced_version_number(const std::shared_ptr<Relation>& relation) const {
            auto it = referenced_version_numbers_.find(relation);
            if (it != referenced_version_numbers_.end())
                return it->second;
            return -1;
        }

        version_numbers_t::const_iterator referenced_version_numbers_begin() const {
            return referenced_version_numbers_.begin();
        }

        version_numbers_t::const_iterator referenced_version_numbers_end() const {
            return referenced_version_numbers_.end();
        }

    private:
#endif

        Tuple(Schema::ptr_t schema_ptr, data_t data, time_t arrived_time):
            type_(DATA),
            schema_ptr_(schema_ptr),
            data_(data),
            arrived_time_(arrived_time) {
#ifdef CURRENTIA_ENABLE_TRANSACTION
            set_hwm(arrived_time);
#endif
        }

        Tuple(Type type): type_(type) {
        }

    public:
        static Tuple::ptr_t create_eos() {
            return Tuple::ptr_t(new Tuple(EOS));
        }

        static time_t get_current_time() {
            static long current_time = 1;
            return current_time++;
        }

        static Tuple::ptr_t create(Schema::ptr_t schema_ptr,
                                   data_t data) {
            return Tuple::create(schema_ptr, data, get_current_time());
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

#ifdef CURRENTIA_ENABLE_TRANSACTION
            ss << "[";
            auto iter = referenced_version_numbers_begin();
            auto iter_end = referenced_version_numbers_end();
            while (iter != iter_end) {
                ss << iter->second;
                if (++iter == iter_end)
                    break;
                ss << ", ";
            }
            ss << "]";
            ss << get_hwm() << std::endl;
#endif

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

        // Already wrapped with Object

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1) {
            data_t data;
            data.push_back(o1);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2, const Object& o3) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            data.push_back(o3);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2, const Object& o3, const Object& o4) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            data.push_back(o3);
            data.push_back(o4);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2, const Object& o3, const Object& o4, const Object& o5) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            data.push_back(o3);
            data.push_back(o4);
            data.push_back(o5);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2, const Object& o3, const Object& o4, const Object& o5, const Object& o6) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            data.push_back(o3);
            data.push_back(o4);
            data.push_back(o5);
            data.push_back(o6);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2, const Object& o3, const Object& o4, const Object& o5, const Object& o6, const Object& o7) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            data.push_back(o3);
            data.push_back(o4);
            data.push_back(o5);
            data.push_back(o6);
            data.push_back(o7);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2, const Object& o3, const Object& o4, const Object& o5, const Object& o6, const Object& o7, const Object& o8) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            data.push_back(o3);
            data.push_back(o4);
            data.push_back(o5);
            data.push_back(o6);
            data.push_back(o7);
            data.push_back(o8);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2, const Object& o3, const Object& o4, const Object& o5, const Object& o6, const Object& o7, const Object& o8, const Object& o9) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            data.push_back(o3);
            data.push_back(o4);
            data.push_back(o5);
            data.push_back(o6);
            data.push_back(o7);
            data.push_back(o8);
            data.push_back(o9);
            return Tuple::create(schema, data);
        }

        static
        Tuple::ptr_t create_easy(const Schema::ptr_t& schema, const Object& o1, const Object& o2, const Object& o3, const Object& o4, const Object& o5, const Object& o6, const Object& o7, const Object& o8, const Object& o9, const Object& o10) {
            data_t data;
            data.push_back(o1);
            data.push_back(o2);
            data.push_back(o3);
            data.push_back(o4);
            data.push_back(o5);
            data.push_back(o6);
            data.push_back(o7);
            data.push_back(o8);
            data.push_back(o9);
            data.push_back(o10);
            return Tuple::create(schema, data);
        }
    };
}

#endif  /* ! CURRENTIA_TUPLE_H_ */
