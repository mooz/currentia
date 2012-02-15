// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_H_
#define CURRENTIA_OPERATOR_H_

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/schema.h"
#include "currentia/core/pointer.h"

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class Operator: private NonCopyable<Operator>,
                    public Pointable<Operator> {
    public:
        // get next tuple from input stream and process
        Tuple::ptr_t next() {
            Tuple::ptr_t result = next_implementation();
            return result;
        }

        virtual Tuple::ptr_t next_implementation() = 0; // blocking operator

        virtual Schema::ptr_t get_output_schema_ptr() = 0;
    };
}

#endif  /* ! CURRENTIA_OPERATOR_H_ */
