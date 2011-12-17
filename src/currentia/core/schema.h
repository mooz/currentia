// -*- c++ -*-

#ifndef CURRENTIA_SCHEMA_H__
#define CURRENTIA_SCHEMA_H__

#include <map>
#include "currentia/core/object.h"

namespace currentia {
    typedef std::map<char*, enum Type> attributes_t;

    class Schema {
        long id;                /* ID of the stream */
        attributes_t attributes;
    };
}

#endif  /* ! CURRENTIA_SCHEMA_H__ */
