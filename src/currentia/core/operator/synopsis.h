// -*- c++ -*-

#ifndef CURRENTIA_SYNOPSIS_H_
#define CURRENTIA_SYNOPSIS_H_

#include <vector>
#include <assert.h>
#include <functional>

#include "currentia/core/operator/operator.h"
#include "currentia/core/relation.h"
#include "currentia/core/thread.h"
#include "currentia/core/tuple.h"
#include "currentia/core/window.h"
#include "currentia/trait/non-copyable.h"

namespace currentia {
    class Synopsis: private NonCopyable<Synopsis> {
    public:
        typedef std::vector<Tuple::ptr_t>::const_iterator const_iterator;
        typedef std::function<void(void)> callback_t;

    private:
        pthread_mutex_t mutex_;
        pthread_cond_t reader_wait_;

        Window window_;
        long index_;
        long newcomer_count_;

        callback_t on_accept_;

        std::vector<Tuple::ptr_t> tuples_;
        std::vector<Tuple::ptr_t> newcomer_tuples_;

        bool window_filled_;

    public:
        Synopsis(Window &window):
            window_(window),
            index_(0),
            newcomer_count_(0),
            on_accept_(NULL),
            tuples_(window.width),
            newcomer_tuples_(window.width),
            window_filled_(false) {
            pthread_mutex_init(&mutex_, NULL);
            pthread_cond_init(&reader_wait_, NULL);
        }

        void enqueue(const Tuple::ptr_t& input_tuple) {
            switch (window_.type) {
            case Window::TUPLE_BASE:
                return enqueue_logical(input_tuple);
            case Window::TIME_BASE:
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
            thread::ScopedLock lock(&mutex_);

            newcomer_tuples_[newcomer_count_] = input_tuple;
            newcomer_count_ = newcomer_count_ + 1 % window_.width;

            if (window_filled_) { // Branch prediction, please!
                if (newcomer_count_ == window_.stride) {
                    accept_newcomers_logical_();
                }
            } else {
                if (newcomer_count_ == window_.width) {
                    accept_newcomers_logical_();
                    window_filled_ = true;
                }
            }
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

        bool has_reference_consistency() {
            // TODO: stop queueing (take lock)

            auto tuple_iter = begin();
            auto tuple_iter_end = end();

            Tuple::ptr_t first_tuple = *tuple_iter++;

            for (; tuple_iter != tuple_iter_end; ++tuple_iter) {
                Tuple::ptr_t non_first_tuple = *tuple_iter; // cost

                auto first_version_iter = first_tuple->referenced_version_numbers_begin();
                auto first_version_iter_end = first_tuple->referenced_version_numbers_end();

                // For all rerations in first tuple's lineage, check
                // if the reference consistency is same for all other
                // relations.
                for (; first_version_iter != first_version_iter_end; ++first_version_iter) {
                    Relation::ptr_t relation = first_version_iter->first;
                    long version = first_version_iter->second;
                    if (non_first_tuple->get_referenced_version_number(relation) != version) {
                        return false;
                    }
                }
            }

            return true;
        }

        std::string get_versions_string() {
            std::stringstream ss;

            auto tuple_iter = begin();
            auto tuple_iter_end = end();

            ss << "(";

            for (; tuple_iter != tuple_iter_end;) {
                Tuple::ptr_t non_first_tuple = *tuple_iter; // cost

                auto version_iter = (*tuple_iter)->referenced_version_numbers_begin();
                auto version_iter_end = (*tuple_iter)->referenced_version_numbers_end();

                for (; version_iter != version_iter_end; ++version_iter) {
                    Relation::ptr_t relation = version_iter->first;
                    long version = version_iter->second;
                    ss << version;
                }

                ++tuple_iter;
                if (tuple_iter == tuple_iter_end)
                    break;
                ss << ", ";
            }

            ss << ")";

            return ss.str();
        }

    private:

        void accept_newcomers_logical_() {
            // TODO: this breaks the order of tuples in `tuples_`
            for (int i = 0; i < newcomer_count_; ++i)
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
