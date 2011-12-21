// -*- c++ -*-

#ifndef CURRENTIA_SCHEMA_H__
#define CURRENTIA_SCHEMA_H__

#include "currentia/core/object.h"
#include "currentia/core/pointer.h"
#include "currentia/core/thread.h"

#include <map>
#include <vector>
#include <string>
#include <cstddef>              // size_t
#include <iostream>
#include <list>

namespace currentia {
    struct Attribute {
        std::string  name;
        Object::Type type;

        Attribute(std::string name, Object::Type type):
            name(name),
            type(type) {
        }
    };

    inline
    static long get_next_schema_id() {
        static long next_schema_id = 1;
        return next_schema_id++;
    }

    class Schema {
    public:
        typedef std::tr1::shared_ptr<Schema> ptr_t;
        // types
        typedef std::vector<Attribute> attributes_t;
        typedef std::map<std::string, int> attributes_index_t;

        // TODO: use builder pattern? (e.g., builder.add_attribute(xx).add_attribute(yy).build())
        // TODO: Schema decides relation. So we need relation name!
        Schema():
            is_schema_freezed_(false) {
            id_ = get_next_schema_id();
            pthread_mutex_init(&schema_lock_, NULL);
        }

        void freeze() {
            pthread_mutex_lock(&schema_lock_);
            is_schema_freezed_ = true;
            pthread_mutex_unlock(&schema_lock_);
        }

        int add_attribute(Attribute& attribute) {
            return add_attribute(attribute.name, attribute.type);
        }

        // TODO: condsider (schema) relation name.
        int add_attribute(std::string name, Object::Type type) {
            pthread_mutex_lock(&schema_lock_);

            if (!is_schema_freezed_)
                attributes_.push_back(Attribute(name, type));

            // TODO: check if this schema already has attribute with given name
            size_t current_size = this->size();

            if (!is_schema_freezed_)
                attributes_index_[name] = current_size - 1;

            pthread_mutex_unlock(&schema_lock_);

            return current_size;
        }

        inline
        int get_attribute_index_by_name(std::string& attribute_name) {
            // assert(has_attribute(attribute_name));
            return attributes_index_[attribute_name];
        }

        inline
        Attribute get_attribute_by_name(std::string& attribute_name) {
            // assert(has_attribute(attribute_name));
            int attribute_index = get_attribute_index_by_name(attribute_name);
            return attributes_[attribute_index];
        }

        inline
        Attribute get_attribute_by_index(int attribute_index) {
            return attributes_[attribute_index];
        }

        bool has_attribute(std::string& name) const {
            return attributes_index_.find(name) != attributes_index_.end();
        }

        size_t size() const {
            return attributes_.size();
        }

        bool validate_data(std::vector<Object>& data) const {
            if (data.size() != this->size())
                return false;

            std::vector<Object>::const_iterator data_iter = data.begin();
            attributes_t::const_iterator attr_iter = attributes_.begin();

            int attr_number = 0;
            for (; data_iter != data.end(); ++data_iter, ++attr_iter) {
                if (data_iter->type != attr_iter->type) {
                    std::cerr <<
                        "schema expected '" << Object::type2string(attr_iter->type) <<
                        "' for column " << attr_number <<
                        ", but got '" << Object::type2string(data_iter->type) << "'." << std::endl;
                    return false;
                }
                attr_number++;
            }

            return true;
        }

        typedef std::list<Schema::ptr_t> schema_ptrs_t;

        friend Schema::ptr_t concat_schemas(Schema::ptr_t schema1_ptr,
                                            Schema::ptr_t schema2_ptr) {
            schema_ptrs_t schemas;

            schemas.push_back(schema1_ptr);
            schemas.push_back(schema2_ptr);

            return concat_schemas(schemas);
        }

        friend Schema::ptr_t concat_schemas(schema_ptrs_t& schema_ptrs) {
            Schema::ptr_t new_schema_ptr = Schema::ptr_t(new Schema());

            schema_ptrs_t::iterator schema_iter = schema_ptrs.begin();

            for (; schema_iter != schema_ptrs.end(); ++schema_iter) {
                Schema::attributes_t attributes = (*schema_iter)->attributes_;
                Schema::attributes_t::iterator attribute_iter = attributes.begin();
                for (; attribute_iter != attributes.end(); ++attribute_iter) {
                    new_schema_ptr->add_attribute(
                        attribute_iter->name + (*schema_iter)->get_id_string(),
                        attribute_iter->type
                    );
                }
            }

            return new_schema_ptr;
        }

        inline
        long get_id() {
            return id_;
        }

        inline
        std::string get_id_string() {
            std::stringstream ss;
            ss << id_;
            return ss.str();
        }

    private:
        long id_;
        // TODO: give relation name
        pthread_mutex_t schema_lock_;
        bool is_schema_freezed_;

        attributes_t attributes_;
        attributes_index_t attributes_index_;
    };
}

#endif  /* ! CURRENTIA_SCHEMA_H__ */
