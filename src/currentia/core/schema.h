// -*- c++ -*-

#ifndef CURRENTIA_SCHEMA_H__
#define CURRENTIA_SCHEMA_H__

#include "currentia/core/object.h"
#include "currentia/core/pointer.h"

#include <map>
#include <vector>
#include <string>
#include <cstddef>              // size_t
#include <iostream>

namespace currentia {
    struct Attribute {
        std::string     name;
        enum ObjectType type;

        Attribute(std::string name, enum ObjectType type):
            name(name),
            type(type) {
        }
    };

    class Schema {
    public:
        typedef std::tr1::shared_ptr<Schema> ptr_t;
        // types
        typedef std::vector<Attribute> attributes_t;
        typedef std::map<std::string, int> attributes_index_t;

    private:
        long id_;                /* ID of the stream */
        // TODO: give relation name

    public:
        // TODO: make them private
        attributes_t attributes_;
        attributes_index_t attributes_index_;

        // TODO: use builder pattern? (e.g., builder.add_attribute(xx).add_attribute(yy).build())
        Schema(long id): id_(id) {
        }

        int add_attribute(std::string name, enum ObjectType type) {
            attributes_.push_back(Attribute(name, type));
            // TODO: check if this schema already has attribute with given name
            size_t current_size = this->size();
            attributes_index_[name] = current_size - 1;

            return current_size;
        }

        bool has_attribute(std::string name) const {
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
                        "schema expected '" << type2string(attr_iter->type) <<
                        "' for column " << attr_number <<
                        ", but got '" << type2string(data_iter->type) << "'." << std::endl;
                    return false;
                }
                attr_number++;
            }

            return true;
        }
    };
}

#endif  /* ! CURRENTIA_SCHEMA_H__ */
