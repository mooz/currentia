// -*- c++ -*-

#ifndef CURRENTIA_STREAM_H__
#define CURRENTIA_STREAM_H__

#include "currentia/core/schema.h"
#include "currentia/core/tuple.h"
#include "currentia/core/pointer.h"

#include <list>
#include <pthread.h>

namespace currentia {
    /* Stream: just a queue for tuples with concurrent access possiblity */
    class Stream {
    public:
        typedef std::tr1::shared_ptr<Stream> ptr_t;

    private:
        Schema::ptr_t schema_ptr_;
        std::list<Tuple::ptr_t> tuple_ptrs_;
        pthread_mutex_t mutex;

    public:
        Stream(Schema::ptr_t schema_ptr):
            schema_ptr_(schema_ptr) {
            pthread_mutex_init(&mutex, NULL);
        }

        void enqueue(Tuple::ptr_t tuple_ptr) {
            pthread_mutex_lock(&mutex);

            tuple_ptrs_.push_front(tuple_ptr);

            pthread_mutex_unlock(&mutex);
        }

        // blocking
        Tuple::ptr_t dequeue() {
            pthread_mutex_lock(&mutex);

            Tuple::ptr_t tuple_ptr = tuple_ptrs_.back();
            tuple_ptrs_.pop_back();

            pthread_mutex_unlock(&mutex);

            return tuple_ptr;
        }
    };
}

#endif  /* ! CURRENTIA_STREAM_H__ */
