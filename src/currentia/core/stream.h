// -*- c++ -*-

#ifndef CURRENTIA_STREAM_H_
#define CURRENTIA_STREAM_H_

#include "currentia/core/schema.h"
#include "currentia/core/tuple.h"
#include "currentia/core/pointer.h"
#include "currentia/core/thread.h"
#include "currentia/trait/non-copyable.h"

#include <list>

namespace currentia {
    /* Stream: just a queue for tuples with concurrent access possiblity */
    class Stream: private NonCopyable<Stream> {
    public:
        typedef std::tr1::shared_ptr<Stream> ptr_t;

        Stream(Schema::ptr_t schema_ptr):
            schema_ptr_(schema_ptr) {
            // initialize values for thread synchronization
            pthread_mutex_init(&mutex_, NULL);
            pthread_cond_init(&reader_wait_, NULL);
        }

        void enqueue(Tuple::ptr_t tuple_ptr) {
            pthread_mutex_lock(&mutex_);

            tuple_ptrs_.push_front(tuple_ptr);
            // tell arrival of a tuple to waiting threads
            pthread_cond_broadcast(&reader_wait_);

            pthread_mutex_unlock(&mutex_);
        }

        // blocking
        Tuple::ptr_t dequeue() {
            return dequeue_timed_wait(NULL);
        }

        // timeout version
        Tuple::ptr_t dequeue_timed_wait(const struct timespec* timeout) {
            pthread_mutex_lock(&mutex_);

            while (tuple_ptrs_.empty()) {
                // wait for the next tuple arrival to the queue
                if (timeout && !pthread_cond_timedwait(&reader_wait_, &mutex_, timeout)) {
                    return Tuple::ptr_t(); // NULL
                } else {
                    pthread_cond_wait(&reader_wait_, &mutex_);
                }
            }

            Tuple::ptr_t tuple_ptr = dequeue_a_tuple_ptr_();

            pthread_mutex_unlock(&mutex_);

            return tuple_ptr;
        }

        // Used by projection operator
        // TODO: returning private pointer is not a good habit
        //       deeply clone?
        inline
        Schema::ptr_t get_schema_ptr() const {
            schema_ptr_->freeze();
            return schema_ptr_;
        }

    private:
        Schema::ptr_t schema_ptr_;

        std::list<Tuple::ptr_t> tuple_ptrs_;

        pthread_mutex_t mutex_;
        pthread_cond_t reader_wait_;

        inline Tuple::ptr_t dequeue_a_tuple_ptr_() {
            Tuple::ptr_t tuple_ptr = tuple_ptrs_.back();
            tuple_ptrs_.pop_back();
            return tuple_ptr;
        }
    };
}

#endif  /* ! CURRENTIA_STREAM_H_ */
