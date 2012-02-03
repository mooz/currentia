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
        virtual Tuple::ptr_t next() = 0; // blocking operator

        virtual Schema::ptr_t get_output_schema_ptr() = 0;
    };
}

#endif  /* ! CURRENTIA_OPERATOR_H_ */
