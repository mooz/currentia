// -*- c++ -*-

#ifndef CURRENTIA_RELATION_H_
#define CURRENTIA_RELATION_H_

#include "currentia/core/schema.h"
#include "currentia/core/tuple.h"
#include "currentia/core/pointer.h"
#include "currentia/core/thread.h"
#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

#include <list>

namespace currentia {
    class Relation: private NonCopyable<Relation>,
                    public Pointable<Relation> {
        Schema::ptr_t schema_ptr_;

        std::list<Tuple::ptr_t> tuple_ptrs_;

        pthread_mutex_t read_mutex_;

    public:
        class Transaction {
        public:
            typedef Transaction* ptr_t;
            virtual void process() = 0;
        };
        friend class Transaction;

        class ScopedLock {
            pthread_mutex_t* read_mutex_;

        public:
            ScopedLock(pthread_mutex_t* mutex):
                read_mutex_(mutex) {
                pthread_mutex_lock(read_mutex_);
            }

            ~ScopedLock() {
                pthread_mutex_unlock(read_mutex_);
            }
        };

        Relation(Schema::ptr_t schema_ptr,
                 // optional
                 std::list<Tuple::ptr_t> tuple_ptrs = std::list<Tuple::ptr_t>()):
            schema_ptr_(schema_ptr),
            tuple_ptrs_(tuple_ptrs_) {
            // initialize recursive mutex
            pthread_mutexattr_t mutex_attribute;
            pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&read_mutex_, &mutex_attribute);
        }

        // Blocking
        void insert(Tuple::ptr_t tuple_ptr) {
            read_write_lock();
            tuple_ptrs_.push_front(tuple_ptr);
            // tell arrival of a tuple to waiting threads
            unlock();
        }

        ScopedLock get_scoped_lock() {
            return ScopedLock(&read_mutex_);
        }

        // TODO: implement read_lock() and write_lock()
        void read_write_lock() {
            pthread_mutex_lock(&read_mutex_);
        }

        void unlock() {
            pthread_mutex_unlock(&read_mutex_);
        }

        // Blocking
        void do_transaction(const Transaction::ptr_t& transaction) {
            read_write_lock();
            transaction->process();
            unlock();
        }

        // Blocking
        Relation::ptr_t copy() {
            read_write_lock();
            std::list<Tuple::ptr_t> new_tuple_ptrs;
            new_tuple_ptrs.insert(new_tuple_ptrs.begin(), tuple_ptrs_.begin(), tuple_ptrs_.end());
            unlock();

            return Relation::ptr_t(new Relation(schema_ptr_, new_tuple_ptrs));
        }

        Schema::ptr_t get_schema_ptr() const {
            schema_ptr_->freeze();
            return schema_ptr_;
        }

        std::list<Tuple::ptr_t>::const_iterator get_tuple_iterator() const {
            return tuple_ptrs_.begin();
        }

        std::list<Tuple::ptr_t>::const_iterator get_tuple_iterator_end() const {
            return tuple_ptrs_.end();
        }
    };
}

#endif  /* ! CURRENTIA_RELATION_H_ */
