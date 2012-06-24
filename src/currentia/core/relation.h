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

        long version_number_;

    public:
        class Transaction {
        public:
            typedef Transaction* ptr_t;
            virtual void process() = 0;
        };
        friend class Transaction;

        Relation(Schema::ptr_t schema_ptr,
                 // optional
                 std::list<Tuple::ptr_t> tuple_ptrs = std::list<Tuple::ptr_t>()):
            schema_ptr_(schema_ptr),
            tuple_ptrs_(tuple_ptrs),
            version_number_(0) {
            // initialize recursive mutex
            pthread_mutexattr_t mutex_attribute;
            pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&read_mutex_, &mutex_attribute);
        }

        // Blocking
        void insert(Tuple::ptr_t tuple_ptr) {
            ScopedLock lock(&read_mutex_);
            tuple_ptrs_.push_front(tuple_ptr);
            // tell arrival of a tuple to waiting threads
        }

        void update() {
            ScopedLock lock(&read_mutex_);
            version_number_++;
        }

        long get_version_number() {
            ScopedLock lock(&read_mutex_);
            long current_version_number = version_number_;

            return current_version_number;
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
            ScopedLock lock(&read_mutex_);
            transaction->process();
        }

        // Blocking
        Relation::ptr_t copy() {
            ScopedLock lock(&read_mutex_);
            std::list<Tuple::ptr_t> new_tuple_ptrs = tuple_ptrs_;

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
