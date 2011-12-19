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

        Schema::ptr_t output_schema_ptr_;

        // get next tuple from input stream and process
        virtual Tuple::ptr_t next() = 0; // blocking operator

        // Operator(Schema::ptr_t& output_schema_ptr, Operator::ptr_t& parent_operator_ptr):
        //     output_schema_ptr_(output_schema_ptr),
        //     parent_operator_ptr_(parent_operator_ptr) {
        // }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_H__ */
