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

    private:
        std::list<process_hook_t> after_process_hook;
        Stream::ptr_t input_stream_;
        Stream::ptr_t output_stream_;

    public:
        virtual ~Operator() = 0;

        // get next tuple from input stream and process
        Tuple::ptr_t next() {
            Tuple::ptr_t result = next_implementation();

            if (result) {
                output_stream_->enqueue(result);
            }

            if (!after_process_hook.empty()) {
                for (std::list<process_hook_t>::iterator process_iterator = after_process_hook.begin();
                     process_iterator != after_process_hook.end();
                     ++process_iterator) {
                    (*process_iterator)(result);
                }
            }

            return result;
        }

        virtual Tuple::ptr_t next_implementation() = 0;

        // Returns the schema of this operator's output stream
        virtual Schema::ptr_t get_output_schema_ptr() = 0;

        // Had been used to achieve pessimistic concurrency control (lock / versioning)
        void add_after_process(process_hook_t hook) {
            after_process_hook.push_back(hook);
        }
    };

    Operator::~Operator() {}
}

#endif  /* ! CURRENTIA_OPERATOR_H_ */
