// -*- c++ -*-

#ifndef CURRENTIA_STREAM_H_
#define CURRENTIA_STREAM_H_

#include "currentia/core/schema.h"
#include "currentia/core/tuple.h"
#include "currentia/core/pointer.h"
#include "currentia/core/thread.h"
#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"
#include "currentia/trait/show.h"

#include <deque>
#include <algorithm>

namespace currentia {
    /* Stream: just a queue for tuples with concurrent access possiblity */
    class Stream: private NonCopyable<Stream>,
                  public Pointable<Stream>,
                  public Show {
    private:
        Schema::ptr_t schema_ptr_;

        typedef std::deque<Tuple::ptr_t> QueueType;
        QueueType tuple_ptrs_;

        mutable pthread_mutex_t mutex_;
        pthread_cond_t reader_wait_;

        QueueType backup_tuple_ptrs_;
        bool do_backup_;

    public:
        explicit
        Stream(Schema::ptr_t schema_ptr):
            schema_ptr_(schema_ptr),
            do_backup_(false) {
            // initialize values for thread synchronization
            pthread_mutexattr_t mutex_attribute;
            pthread_mutexattr_init(&mutex_attribute);
            pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&mutex_, &mutex_attribute);
            pthread_cond_init(&reader_wait_, NULL);
        }

        static Stream::ptr_t from_schema(const Schema::ptr_t& schema) {
            return Stream::ptr_t(new Stream(schema));
        }

        // TODO: not exception safe
        void enqueue(Tuple::ptr_t tuple_ptr) {
            thread::ScopedLock lock(&mutex_);
            tuple_ptrs_.push_front(tuple_ptr);
            if (do_backup_)
                backup_tuple_ptrs_.push_front(tuple_ptr);
            // tell arrival of a tuple to waiting threads
            pthread_cond_broadcast(&reader_wait_);
        }

        // blocking
        Tuple::ptr_t dequeue() {
            return dequeue_timed_wait(NULL);
        }

        // TODO: not exception safe
        // timeout version
        Tuple::ptr_t dequeue_timed_wait(const struct timespec* timeout) {
            thread::ScopedLock lock(&mutex_);

            while (tuple_ptrs_.empty()) {
                // wait for the next tuple arrival to the queue
                if (timeout && !pthread_cond_timedwait(&reader_wait_, &mutex_, timeout)) {
                    return Tuple::ptr_t(); // NULL
                } else {
                    pthread_cond_wait(&reader_wait_, &mutex_);
                }
            }

            Tuple::ptr_t tuple_ptr = dequeue_a_tuple_ptr_();

            return tuple_ptr;
        }

        // Dequeue an elemen from stream. If the stream is empty, returns NULL.
        Tuple::ptr_t non_blocking_dequeue() {
            thread::ScopedLock lock(&mutex_);

            if (tuple_ptrs_.empty())
                return Tuple::ptr_t();

            Tuple::ptr_t tuple_ptr = dequeue_a_tuple_ptr_();

            return tuple_ptr;
        }

        void lock() const {
            pthread_mutex_lock(&mutex_);
        }

        void unlock() const {
            pthread_mutex_unlock(&mutex_);
        }

        // Used by projection operator
        // TODO: returning private pointer is not a good habit
        //       deeply clone?
        inline
        Schema::ptr_t get_schema() const {
            schema_ptr_->freeze();
            return schema_ptr_;
        }

        void clear() {
            thread::ScopedLock lock(&mutex_);

#if 0
            bool stream_has_system_message = false;
            {
                auto iter = tuple_ptrs_.begin();
                auto iter_end = tuple_ptrs_.end();
                for (; iter != iter_end; ++iter) {
                    if ((*iter)->is_system_message()) {
                        stream_has_system_message = true;
                        break;
                    }
                }
            }

            if (stream_has_system_message) {
                bool backup_has_system_message;
                {
                    auto iter = backup_tuple_ptrs_.begin();
                    auto iter_end = backup_tuple_ptrs_.end();
                    for (; iter != iter_end; ++iter) {
                        if ((*iter)->is_system_message()) {
                            backup_has_system_message = true;
                            break;
                        }
                    }
                }
                if (backup_has_system_message) {
                    std::clog << "OK, backup has system message!" << std::endl;
                } else {
                    std::clog << "Oh shit!, backup doesn't have system message!" << std::endl;
                    exit(1);
                }
            }
#endif

            tuple_ptrs_.clear();
        }

        void insert_head(const Stream::ptr_t& another_stream) {
            thread::ScopedLock lock(&mutex_);
            thread::ScopedLock another_lock(&(another_stream->mutex_));
            tuple_ptrs_.insert(tuple_ptrs_.end(),
                               another_stream->tuple_ptrs_.begin(),
                               another_stream->tuple_ptrs_.end());
        }

        // TODO: take all iterable
        void insert_head(const QueueType& another_tuples) {
            thread::ScopedLock lock(&mutex_);
            tuple_ptrs_.insert(tuple_ptrs_.end(),
                               another_tuples.begin(),
                               another_tuples.end());
        }

        std::string toString() const {
            thread::ScopedLock lock(&mutex_);

            std::stringstream ss;
            auto iter = tuple_ptrs_.begin();
            auto iter_end = tuple_ptrs_.end();
            while (iter != iter_end) {
                ss << (*iter)->toString();
                iter++;
                if (iter == iter_end)
                    break;
                ss << ", ";
            }

            return ss.str();
        }

    private:
        inline Tuple::ptr_t dequeue_a_tuple_ptr_() {
            Tuple::ptr_t tuple_ptr = tuple_ptrs_.back();
            tuple_ptrs_.pop_back();
            return tuple_ptr;
        }

        // Backup
    public:
        void set_backup_state(bool backup) {
            thread::ScopedLock lock(&mutex_);
            do_backup_ = backup;
        }

        void recover_from_backup() {
            insert_head(backup_tuple_ptrs_);
        }

        struct IsOlderThan {
            int base_time;
            IsOlderThan(int base_time): base_time(base_time) {
            }
            bool operator ()(const Tuple::ptr_t& tuple) const {
                // std::clog << "If " << tuple->get_arrived_time() << " is older than " << base_time << std::endl;
                return !tuple->is_system_message() && tuple->get_arrived_time() < base_time;
            }
        };

        void evict_backup_tuples_older_than(time_t lwm) {
            thread::ScopedLock lock(&mutex_);
            backup_tuple_ptrs_.erase(
                remove_if(backup_tuple_ptrs_.begin(),
                          backup_tuple_ptrs_.end(), IsOlderThan(lwm)),
                backup_tuple_ptrs_.end()
            );
        }

        QueueType::const_iterator backup_tuple_begin() {
            return backup_tuple_ptrs_.begin();
        }

        QueueType::const_iterator backup_tuple_end() {
            return backup_tuple_ptrs_.end();
        }
    };
}

#endif  /* ! CURRENTIA_STREAM_H_ */
