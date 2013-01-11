// -*- c++ -*-

#ifndef CURRENTIA_SYNOPSIS_H_
#define CURRENTIA_SYNOPSIS_H_

#include <deque>
#include <assert.h>
#include <functional>

#include "currentia/core/operator/operator.h"
#include "currentia/core/relation.h"
#include "currentia/core/thread.h"
#include "currentia/core/tuple.h"
#include "currentia/core/window.h"
#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"
#include "currentia/trait/show.h"

namespace currentia {
    class Synopsis: private NonCopyable<Synopsis>,
                    public Pointable<Synopsis>,
                    public Show {
    public:
        typedef std::function<void(void)> callback_t;
        typedef std::deque<Tuple::ptr_t>::const_iterator const_iterator;
        typedef std::deque<Tuple::ptr_t>::iterator iterator;

    protected:
        pthread_mutex_t mutex_;
        pthread_cond_t reader_wait_;

        Window window_;
        callback_t on_accept_;

        Synopsis(Window &window):
            window_(window),
            on_accept_(NULL) {
            pthread_mutex_init(&mutex_, NULL);
            pthread_cond_init(&reader_wait_, NULL);
        }

    public:
        virtual ~Synopsis() = 0;

        virtual void reset() = 0;

        virtual void enqueue(const Tuple::ptr_t& input_tuple) = 0;

        virtual Synopsis::const_iterator begin() const = 0;
        virtual Synopsis::const_iterator end() const = 0;

        virtual Tuple::ptr_t get_window_beginning_tuple() const = 0;
        virtual Tuple::ptr_t get_latest_tuple() const = 0;

        std::string toString() const {
            std::stringstream ss;
            Synopsis::const_iterator iter = begin();

            int i = 0;
            for (; iter != end(); ++iter) {
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

                // For all relations in first tuple's lineage, check
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

        time_t get_lwm() {
            auto tuple_iter = begin();
            auto tuple_iter_end = end();

            if (tuple_iter == tuple_iter_end)
                throw "No tuples in the synopsis but lwm is requested";

            time_t lwm = (*tuple_iter++)->get_lwm();
            for (; tuple_iter != tuple_iter_end; ++tuple_iter) {
                lwm = std::min(lwm, (*tuple_iter)->get_lwm());
            }

            return lwm;
        }

        std::string get_versions_string() {
            std::stringstream ss;

            auto tuple_iter = begin();
            auto tuple_iter_end = end();

            ss << "(";

            for (; tuple_iter != tuple_iter_end;) {
                Tuple::ptr_t tuple = *tuple_iter; // cost

                auto version_iter = (*tuple_iter)->referenced_version_numbers_begin();
                auto version_iter_end = (*tuple_iter)->referenced_version_numbers_end();

                for (; version_iter != version_iter_end; ++version_iter) {
                    Relation::ptr_t relation = version_iter->first;
                    long version = version_iter->second;
                    ss << version;
                    ss << "<" << tuple->get_lwm() << ">";
                }

                ++tuple_iter;
                if (tuple_iter == tuple_iter_end)
                    break;
                ss << ", ";
            }

            ss << ")[" << get_lwm() << "]";

            return ss.str();
        }

    protected:
        void acceptance_notification_() {
            if (on_accept_)
                on_accept_();
        }
    };
    Synopsis::~Synopsis() {}

    class TupleBaseSynopsis: private NonCopyable<TupleBaseSynopsis>,
                             public Synopsis {
    private:
        long index_;
        long newcomer_count_;

        std::deque<Tuple::ptr_t> tuples_;
        std::deque<Tuple::ptr_t> newcomer_tuples_;

        bool window_filled_;
        int window_beginning_;

    public:
        TupleBaseSynopsis(Window &window):
            Synopsis(window),
            tuples_(window.width),
            newcomer_tuples_(window.width) {
            reset();
        }

        void reset() {
            index_ = 0;
            newcomer_count_ = 0;
            window_filled_ = false;
            window_beginning_ = 0;
        }

        void enqueue(const Tuple::ptr_t& input_tuple) {
            thread::ScopedLock lock(&mutex_);

            newcomer_tuples_[newcomer_count_++] = input_tuple;

            if (window_filled_) { // Branch prediction, please!
                // After 1st acceptance
                if (newcomer_count_ == window_.stride) {
                    accept_newcomers_logical_(newcomer_count_);
                }
            } else {
                // Before 1st acceptance
                if (newcomer_count_ == window_.width) {
                    accept_newcomers_logical_(newcomer_count_);
                }
            }
        }

        Synopsis::const_iterator begin() const {
            return tuples_.begin();
        }

        Synopsis::const_iterator end() const {
            return tuples_.end();
        }

        Tuple::ptr_t get_window_beginning_tuple() const {
            return tuples_[window_beginning_];
        }

        Tuple::ptr_t get_latest_tuple() const {
            return tuples_[index_];
        }

        std::string toString() const {
            return "(TUPLE-BASE)" + Synopsis::toString();
        }

    private:

        // Logical (Tuple-base window)
        void accept_newcomers_logical_(long number_of_newcomer) {
            // TODO: this breaks the order of tuples in `tuples_`
            int current_window_beginning = peek_current_index_() - window_.width;
            if (current_window_beginning < 0)
                current_window_beginning += window_.width;
            window_beginning_ = (current_window_beginning + window_.stride) % window_.width;

            for (int i = 0; i < number_of_newcomer; ++i) {
                tuples_[get_current_index_and_increment_()] = newcomer_tuples_[i];
            }

            newcomer_count_ = 0;
            window_filled_ = true;

            acceptance_notification_();
        }

        inline long get_current_index_and_increment_() {
            long current_index = index_;
            index_ = (index_ + 1) % window_.width;
            return current_index;
        }

        inline long peek_current_index_() {
            return index_;
        }
    };

#ifdef CURRENTIA_ENABLE_TIME_BASED_WINDOW
#define COMPARE_TIME(A, CMP, B) (time::timeval_difference_msec((A), (B)) CMP 0)
    class TimeBaseSynopsis: private NonCopyable<TimeBaseSynopsis>,
                            public Synopsis {
    private:
        struct timeval window_beginning_time_;
        struct timeval window_end_time_;

        bool initialized_;

        std::deque<Tuple::ptr_t> tuples_;

    public:
        TimeBaseSynopsis(Window &window):
            Synopsis(window),
            initialized_(false) {
            reset();
        }

        void reset() {
        }

        Synopsis::const_iterator begin() const {
            return tuples_.begin();
        }

        Synopsis::const_iterator end() const {
            return tuples_.end();
        }

        Tuple::ptr_t get_window_beginning_tuple() const {
            return *begin();
        }

        Tuple::ptr_t get_latest_tuple() const {
            return *(end() - 1);
        }

        std::string toString() const {
            return "(TIME-BASE)" + Synopsis::toString();
        }

        void enqueue(const Tuple::ptr_t& input_tuple) {
            thread::ScopedLock lock(&mutex_);

            if (!initialized_) {
                // For the first enqueue call
                initialized_ = true;
                window_beginning_time_ = input_tuple->get_real_arrived_time();
                sync_window_end_time_with_beginning_time_();
            }

            if (COMPARE_TIME(input_tuple->get_real_arrived_time(), <=, window_end_time_)) {
                // (1) input_tuple is in the current window
                tuples_.push_back(input_tuple);
            } else {
                // (2) input_tuple isn't in the current window
                // Evaluate sliding windows that doesn't include the input_tuple.
                do {
                    // TODO: operator may be evaluated multiple times
                    // this may break the correctness of the constraint-scheduler
                    if (begin() != end()) {
                        // When at least one tuple exists in the current window, evaluate the operator
                        acceptance_notification_();
                    }
                    slide_window_();
                    evict_expired_tuples_();
                } while (COMPARE_TIME(input_tuple->get_real_arrived_time(), >, window_end_time_));
                // Then, push the input_tuple. If windows are
                // landmark-windows (slide > width), the input_tuple
                // may not be contained in the next window.
                if (COMPARE_TIME(input_tuple->get_real_arrived_time(), >=, window_beginning_time_)) {
                    tuples_.push_back(input_tuple);
                }
                evict_expired_tuples_();
            }
        }

    private:
        void sync_window_end_time_with_beginning_time_() {
            window_end_time_ = window_beginning_time_;
            time::timeval_add_msec(&window_end_time_, window_.width);
        }

        void set_window_beginning_time_(const struct timeval& new_beginning_time) {
            window_beginning_time_ = new_beginning_time;
            sync_window_end_time_with_beginning_time_();
        }

        void slide_window_() {
            time::timeval_add_msec(&window_beginning_time_, window_.stride);
            sync_window_end_time_with_beginning_time_();
        }

        void evict_expired_tuples_() {
            for (auto iter = tuples_.begin(); iter != tuples_.end();) {
                if (COMPARE_TIME((*iter)->get_real_arrived_time(), <, window_beginning_time_)) {
                    iter = tuples_.erase(iter);
                } else {
                    ++iter;
                }
            }
        }
    };
#undef COMPARE_TIME
#endif

    Synopsis::ptr_t create_synopsis_from_window(Window& window) {
        switch (window.type) {
        case Window::TUPLE_BASE:
            return Synopsis::ptr_t(new TupleBaseSynopsis(window));
        case Window::TIME_BASE:
#ifdef CURRENTIA_ENABLE_TIME_BASED_WINDOW
            return Synopsis::ptr_t(new TimeBaseSynopsis(window));
#else
            throw "Time-based windows are not enabled. Specify -DCURRENTIA_ENABLE_TIME_BASED_WINDOW.";
#endif
        }
    }
}

#endif  /* ! CURRENTIA_SYNOPSIS_H_ */
