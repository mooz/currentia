// -*- c++ -*-

#ifndef CURRENTIA_THREAD_H_
#define CURRENTIA_THREAD_H_

// TODO: more portable (platform independent)
#include <pthread.h>

namespace currentia {
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
}

#endif  /* ! CURRENTIA_THREAD_H_ */
