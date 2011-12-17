// -*- c++ -*-

#ifndef CURRENTIA_TUPLE_H__
#define CURRENTIA_TUPLE_H__

#include "currentia/core/schema.h"
#include "currentia/core/object.h"

#include <vector>
#include <tr1/memory>               // shared_ptr

namespace currentia {
    class Tuple {
        typedef std::tr1::shared_ptr<Schema> schema_ptr_t;
        typedef std::tr1::shared_ptr<std::vector<Object> > data_ptr_t;

    private:
        // TODO: make them immutable
        schema_ptr_t schema_;
        data_ptr_t data_;

    public:
        Tuple(schema_ptr_t schema, data_ptr_t data) {
            schema_ = schema;
            data_ = data;
        }
    };
}

#endif  /* ! CURRENTIA_TUPLE_H__ */
