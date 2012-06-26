// -*- c++ -*-

#ifndef CURRENTIA_THREAD_H_
#define CURRENTIA_THREAD_H_

// TODO: more portable (platform independent)
#include <pthread.h>
#include <sched.h>

namespace currentia {
    namespace thread {
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

        // The scheduler_yield() function shall force the running thread to
        // relinquish the processor until  it again becomes the head of
        // its thread list. It takes no arguments.
        //
        // The scheduler_yield()  function shall  return 0 if  it completes
        // successfully; otherwise,  it shall return a value  of -1 and
        // set errno to indicate the error.
        inline
        int scheduler_yield() {
            return sched_yield();
        }
    }
}

#endif  /* ! CURRENTIA_THREAD_H_ */
