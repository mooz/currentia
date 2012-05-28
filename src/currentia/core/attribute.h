// -*- c++ -*-

#ifndef CURRENTIA_ATTRIBUTE_H_
#define CURRENTIA_ATTRIBUTE_H_

#include "currentia/core/object.h"
#include "currentia/trait/printable.h"

namespace currentia {
    struct Attribute : public Printable<Attribute> {
        std::string  name;
        Object::Type type;

        Attribute(std::string name, Object::Type type):
            name(name),
            type(type) {
        }

        std::string toString() const {
            return "<" + name + " " + Object::type_to_string(type) + ">";
        }
    };

    // TODO: is this header file is suitable for placing this class?
    struct AttributeIdentifier : public Printable<AttributeIdentifier> {
        std::string name;
        std::string stream_name;

        AttributeIdentifier(const std::string& name, const std::string& stream_name):
            name(name),
            stream_name(stream_name) {
        }

        std::string toString() const {
            return stream_name + "." + name;
        }
    };
}

#endif  /* ! CURRENTIA_ATTRIBUTE_H_ */
