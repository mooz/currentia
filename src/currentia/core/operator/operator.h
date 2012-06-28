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

    protected:
        // Since the number of input streams varies, the base class
        // Operator don't have input_stream_, though in this
        // implementation, we assume the number of output stream is
        // fixed to one (a physical plan represents a tree). Note that
        // each operator implementation *MUST* create an instance of
        // output stream.
        Stream::ptr_t output_stream_;

    public:
        virtual ~Operator() = 0;

        // get next tuple from input stream and process
        Tuple::ptr_t process_next() {
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

        virtual Tuple::ptr_t next_implementation() = 0;

        // Returns the schema of this operator's output stream
        Schema::ptr_t get_output_schema_ptr() {
            return get_output_stream()->get_schema_ptr();
        }

        void set_output_stream(Stream::ptr_t output_stream) {
            output_stream_ = output_stream;
        }

        Stream::ptr_t get_output_stream() {
            return output_stream_;
        }

        void output_tuple(const Tuple::ptr_t& tuple) {
            output_stream_->enqueue(tuple);
        }

        // Had been used to achieve pessimistic concurrency control (lock / versioning)
        void add_after_process(process_hook_t hook) {
            after_process_hook.push_back(hook);
        }

        virtual std::string toString() const = 0;
    };

    Operator::~Operator() {}
}

#endif  /* ! CURRENTIA_OPERATOR_H_ */
