// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PROCESSOR_H_
#define CURRENTIA_QUERY_PROCESSOR_H_

#include "currentia/core/thread.h"
#include "currentia/core/scheduler/abstract-scheduler.h"

namespace currentia {
    class QueryProcessor : public thread::Runnable {
        AbstractScheduler::ptr_t scheduler_;

    public:
        explicit QueryProcessor(const AbstractScheduler::ptr_t& scheduler):
            scheduler_(scheduler) {
        }

        void run() {
            while (!stopped()) {
                scheduler_->wake_up();
                thread::scheduler_yield();
            }

#ifdef CURRENTIA_DEBUG
            std::cout << "QueryProcessor Finished" << std::endl;
#endif
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_PROCESSOR_H_ */
