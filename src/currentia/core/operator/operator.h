// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_H_
#define CURRENTIA_OPERATOR_H_

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/schema.h"
#include "currentia/core/pointer.h"

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"
#include "currentia/trait/show.h"

namespace currentia {
    class Operator: private NonCopyable<Operator>,
                    public Pointable<Operator>,
                    public Show {
    public:
#ifdef CURRENTIA_ENABLE_TRANSACTION
        // Concurrency Control Mode
        enum CCMode {
            NONE                 = 0,
            OPTIMISTIC           = 1,
            PESSIMISTIC_2PL      = 2,
            PESSIMISTIC_SNAPSHOT = 3
            // PESSIMISTIC_* & 2 != 0
        };

        static const int PESSIMISTIC_MASK = 2;
#endif

    protected:
        // Since the number of input streams varies, the base class
        // Operator don't have input_stream_, though in this
        // implementation, we assume the number of output stream is
        // fixed to one (a physical plan represents a tree). Note that
        // each operator implementation *MUST* create an instance of
        // output stream.
        Stream::ptr_t output_stream_;
        bool is_commit_operator_;
        bool is_redo_area_leaf_;

        long evaluation_count_;

#ifdef CURRENTIA_ENABLE_TRANSACTION
        enum CCMode cc_mode_;
#endif

    public:
        Operator():
            is_redo_area_leaf_(false),
            evaluation_count_(0) {
            set_is_commit_operator(false);
        }
        virtual ~Operator() = 0;

        // get next tuple from input stream and process
        void process_next(int batch_count = 1) {
            if (batch_count == 1) {
                next_implementation();
            } else {
                for (int i = 0; i < batch_count; ++i) {
                    next_implementation();
                }
            }
        }

        virtual void next_implementation() = 0;

        virtual void reset() {
        }

        void set_is_commit_operator(bool is_commit_operator) {
            is_commit_operator_ = is_commit_operator;
        }

        bool is_commit_operator() const {
            return is_commit_operator_;
        }

        void set_is_redo_area_leaf(bool is_redo_area_leaf) {
            is_redo_area_leaf_ = is_redo_area_leaf;
        }

        bool is_redo_area_leaf() {
            return is_redo_area_leaf_;
        }

        long get_evaluation_count() const {
            return evaluation_count_;
        }

#ifdef CURRENTIA_ENABLE_TRANSACTION
        void set_cc_mode(enum CCMode cc_mode) {
            cc_mode_ = cc_mode;
        }

        enum CCMode get_cc_mode() const {
            return cc_mode_;
        }

        bool in_pessimistic_cc() const {
            return cc_mode_ & PESSIMISTIC_MASK;
        }
#endif

        // Returns the schema of this operator's output stream
        Schema::ptr_t get_output_schema_ptr() const {
            return get_output_stream()->get_schema();
        }

        void set_output_stream(Stream::ptr_t output_stream) {
            output_stream_ = output_stream;
        }

        Stream::ptr_t get_output_stream() const {
            return output_stream_;
        }

        void output_tuple(const Tuple::ptr_t& tuple) {
            // if (tuple->is_system_message()) {
            //     std::clog << "Operator " << get_name() << " output EOS : " << output_stream_ << std::endl;
            //     output_stream_->enqueue(tuple);
            //     std::clog << "Operator done!" << std::endl;
            //     std::clog << output_stream_->toString() << std::endl;
            // } else {
            output_stream_->enqueue(tuple);
            // }
        }

        virtual std::string get_name() const = 0;
    };

    Operator::~Operator() {}
}

#endif  /* ! CURRENTIA_OPERATOR_H_ */
