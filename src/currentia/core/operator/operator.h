// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_H__
#define CURRENTIA_OPERATOR_H__

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/schema.h"
#include "currentia/core/pointer.h"

namespace currentia {
    class Operator {
    public:
        typedef std::tr1::shared_ptr<Operator> ptr_t;

        // get next tuple from input stream and process
        virtual Tuple::ptr_t next() = 0; // blocking operator

        virtual inline Schema::ptr_t get_output_schema_ptr() = 0;
    };
}

#endif  /* ! CURRENTIA_OPERATOR_H__ */
