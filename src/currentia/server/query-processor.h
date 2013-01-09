// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PROCESSOR_H_
#define CURRENTIA_QUERY_PROCESSOR_H_

#include "currentia/core/thread.h"
#include "currentia/core/scheduler/abstract-scheduler.h"

namespace currentia {
    class QueryProcessor : public thread::Runnable {
        AbstractScheduler* scheduler_;

    public:
        explicit QueryProcessor(AbstractScheduler* scheduler):
            scheduler_(scheduler) {
        }

        void run() {
            while (!stopped()) {
                // std::cout << "Wake up scheduler" << std::endl;
                scheduler_->wake_up();
                thread::scheduler_yield();
            }

            std::cout << "QueryProcessor Finished" << std::endl;
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_PROCESSOR_H_ */
