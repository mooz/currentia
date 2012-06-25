// -*- c++ -*-

#ifndef CURRENTIA_SYNOPSIS_H_
#define CURRENTIA_SYNOPSIS_H_

#include <vector>
#include <assert.h>
#include <tr1/functional>

#include "currentia/core/operator/operator.h"
#include "currentia/core/thread.h"
#include "currentia/core/tuple.h"
#include "currentia/core/window.h"
#include "currentia/trait/non-copyable.h"

namespace currentia {
    class Synopsis: private NonCopyable<Synopsis> {
    public:
        typedef std::vector<Tuple::ptr_t>::const_iterator const_iterator;
        typedef std::tr1::function<void(void)> callback_t;

    private:
        pthread_mutex_t mutex_;
        pthread_cond_t reader_wait_;

        std::vector<Tuple::ptr_t> tuples_;
        std::vector<Tuple::ptr_t> newcomer_tuples_;
        Window window_;
        long index_;
        long newcomer_count_;

        callback_t on_accept_;

    public:
        Synopsis(Window &window):
            window_(window),
            index_(0),
            newcomer_count_(0),
            on_accept_(NULL) {
            pthread_mutex_init(&mutex_, NULL);
            pthread_cond_init(&reader_wait_, NULL);
        }

        void enqueue(const Tuple::ptr_t& input_tuple) {
            switch (window_.type) {
            case Window::LOGICAL:
                return enqueue_logical(input_tuple);
            case Window::PHYSICAL:
                assert(false);  // TODO: implement physical operator
                break;
            }
        }

        // Keep tuples in `newcomer_tuples_` until the count of
        // newcomers reaches the stride of window. If the count
        // reaches, evict n-oldest tuples (n = window_.stride)
        // from `tuples_` and enqueue the tuples in
        // `newcomer_tuples_`.  After that, call `on_accept`
        // handler to execute specific action (e.g., window-join,
        // aggregation).
        void enqueue_logical(const Tuple::ptr_t& input_tuple) {
            ScopedLock lock(&mutex_);

            newcomer_tuples_[newcomer_count_] = input_tuple;
            newcomer_count_++;
            if (newcomer_count_ == window_.stride)
                accept_newcomers_logical_();
        }

        // read tuples to prepare for next join
        void read_next_tuples(const Operator::ptr_t& target_operator) {
            switch (window_.type) {
            case Window::TUPLE_BASE:
                return read_next_tuples_logical(target_operator);
            case Window::TIME_BASE:
                // TODO: support physical window (is it possible in pull-style processing?)
                return read_next_tuples_logical(target_operator);
            }
        }

        inline void read_next_tuples_logical(const Operator::ptr_t& target_operator) {
            long tuples_count = tuples_.size();
            long read_count;

            if (tuples_count != window_.width) {
                // this is the first time
                read_count = window_.width;
                tuples_.resize(window_.width);
            } else {
                read_count = window_.stride; // read a tuple ${stride of sliding window} times
            }

            for (int i = 0; i < read_count; ++i) {
                tuples_[get_next_index_()] = target_operator->next();
            }
        }

        inline void read_next_tuples_physical(const Operator::ptr_t& target_operator) const {
            // not implemented yet
        }

        inline std::vector<Tuple::ptr_t>::const_iterator begin() const {
            return tuples_.begin();
        }

        inline std::vector<Tuple::ptr_t>::const_iterator end() const {
            return tuples_.end();
        }

        std::string toString() const {
            std::stringstream ss;
            std::vector<Tuple::ptr_t>::const_iterator iter = tuples_.begin();

            int i = 0;
            for (; iter < tuples_.end(); ++iter) {
                ss << "[" << i++ << "] " << (*iter)->toString() << std::endl;
            }

            return ss.str();
        }

        void set_on_accept(callback_t on_accept) {
            on_accept_ = on_accept;
        }

    private:

        void accept_newcomers_logical_() {
            assert(newcomer_count_ == window_.stride);

            // TODO: this breaks the order of tuples in `tuples_`
            for (int i = 0; i < window_.stride; ++i)
                tuples_[get_next_index_()] = newcomer_tuples_[i];

            newcomer_count_ = 0;

            if (on_accept_)
                on_accept_();
        }

        inline long get_next_index_() {
            return index_++ % window_.width;
        }
    };
}

#endif  /* ! CURRENTIA_SYNOPSIS_H_ */
