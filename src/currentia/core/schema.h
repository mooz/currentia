// -*- c++ -*-

#ifndef CURRENTIA_SCHEMA_H_
#define CURRENTIA_SCHEMA_H_

#include "currentia/core/object.h"
#include "currentia/core/pointer.h"
#include "currentia/core/thread.h"
#include "currentia/core/attribute.h"

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/printable.h"
#include "currentia/trait/pointable.h"
#include "currentia/trait/show.h"

#include <map>
#include <vector>
#include <string>
#include <cstddef>              // size_t
#include <iostream>
#include <list>

namespace currentia {
    inline
    static long get_next_schema_id() {
        static long next_schema_id = 1;
        return next_schema_id++;
    }

    class Schema: private NonCopyable<Schema>,
                  public Pointable<Schema>,
                  public Show {
    public:
        // types
        typedef std::vector<Attribute> attributes_t;
        typedef std::map<std::string, int> attributes_index_t;

    private:
        long id_;
        // TODO: give relation name
        pthread_mutex_t schema_lock_;
        bool is_schema_freezed_;

        attributes_t attributes_;
        attributes_index_t attributes_index_;

    public:
        // TODO: use builder pattern? (e.g., builder.add_attribute(xx).add_attribute(yy).build())
        // TODO: Schema decides relation. So we need relation name!
        Schema():
            id_(get_next_schema_id()),
            is_schema_freezed_(false) {
            pthread_mutex_init(&schema_lock_, NULL);
        }

        static Schema::ptr_t from_attributes(const std::list<Attribute>& attributes) {
            Schema* schema = new Schema();
            std::list<Attribute>::const_iterator attribute_iter = attributes.begin();
            for (; attribute_iter != attributes.end(); ++attribute_iter)
                schema->add_attribute(*attribute_iter);
            schema->freeze();
            return Schema::ptr_t(schema);
        }

        static Schema::ptr_t from_attribute_pointers(const std::list<Attribute*>& attributes) {
            Schema* schema = new Schema();
            std::list<Attribute*>::const_iterator attribute_iter = attributes.begin();
            for (; attribute_iter != attributes.end(); ++attribute_iter)
                schema->add_attribute(**attribute_iter);
            schema->freeze();
            return Schema::ptr_t(schema);
        }

        void freeze() {
            thread::ScopedLock lock(&schema_lock_);
            is_schema_freezed_ = true;
        }

        int add_attribute(const Attribute& attribute) {
            return add_attribute(attribute.name, attribute.type);
        }

        // TODO: condsider (schema) relation name.
        int add_attribute(const std::string& name, Object::Type type) {
            thread::ScopedLock lock(&schema_lock_);

            if (!is_schema_freezed_)
                attributes_.push_back(Attribute(name, type));

            // TODO: check if this schema already has attribute with given name
            size_t current_size = this->size();

            if (!is_schema_freezed_)
                attributes_index_[name] = current_size - 1;

            return current_size;
        }

        inline
        int get_attribute_index_by_name(const std::string& attribute_name) const {
            // map access with [] breaks const'ness of method
            attributes_index_t::const_iterator found = attributes_index_.find(attribute_name);
            return found != attributes_index_.end() ? found->second : -1;
        }

        inline
        Attribute get_attribute_by_name(const std::string& attribute_name) const {
            // assert(has_attribute(attribute_name));
            int attribute_index = get_attribute_index_by_name(attribute_name);
            assert_has_attribute_(attribute_index);
            return attributes_[attribute_index];
        }

        inline
        Attribute get_attribute_by_index(int attribute_index) const {
            assert_has_attribute_(attribute_index);
            return attributes_[attribute_index];
        }

        bool has_attribute(const std::string& name) const {
            return attributes_index_.find(name) != attributes_index_.end();
        }

        size_t size() const {
            return attributes_.size();
        }

        bool validate_data(const std::vector<Object>& data) const {
            if (data.size() != this->size())
                return false;

            std::vector<Object>::const_iterator data_iter = data.begin();
            attributes_t::const_iterator attr_iter = attributes_.begin();

            int attr_number = 0;
            for (; data_iter != data.end(); ++data_iter, ++attr_iter) {
                if (data_iter->get_type() != attr_iter->type) {
                    std::cerr <<
                        "schema expected '" << Object::type_to_string(attr_iter->type) <<
                        "' for column " << attr_number <<
                        ", but got '" << Object::type_to_string(data_iter->get_type()) << "'." << std::endl;
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
                        attribute_iter->name,
                        attribute_iter->type
                    );
                }
            }

            return new_schema_ptr;
        }

        long get_id() const {
            return id_;
        }

        std::string get_id_string() const {
            std::stringstream ss;
            ss << id_;
            return ss.str();
        }

        std::string toString() const {
            std::stringstream ss;

            ss << "[ ";
            Schema::attributes_t::const_iterator attributes_iter = attributes_.begin();
            Schema::attributes_t::const_iterator attributes_iter_end = attributes_.end();
            for (; attributes_iter != attributes_iter_end; ++attributes_iter) {
                ss << attributes_iter->toString();
                if (attributes_iter + 1 != attributes_iter_end)
                    ss << ", ";
            }
            ss << " ]";
            ss << "#" << id_;

            return ss.str();
        }

    private:
        void assert_has_attribute_(int index) const {
            if (index < 0 || static_cast<unsigned int>(index) >= attributes_.size())
                throw "This schema does not have a requested attribute";
        }
    };
}

#endif  /* ! CURRENTIA_SCHEMA_H_ */
