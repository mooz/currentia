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
        typedef void (*process_hook_t)(const Tuple::ptr_t&);

        // get next tuple from input stream and process
        Tuple::ptr_t next() {
            Tuple::ptr_t result = next_implementation();

            if (!after_process_hook.empty()) {
                for (std::list<process_hook_t>::iterator process_iterator = after_process_hook.begin();
                     process_iterator != after_process_hook.end();
                     ++process_iterator) {
                    (*process_iterator)(result);
                }
            }

            return result;
        }

        virtual Tuple::ptr_t next_implementation() = 0; // blocking operator

        virtual Schema::ptr_t get_output_schema_ptr() = 0;

        void add_after_process(process_hook_t hook) {
            after_process_hook.push_back(hook);
        }

    private:
        std::list<process_hook_t> after_process_hook;
    };
}

#endif  /* ! CURRENTIA_OPERATOR_H_ */
