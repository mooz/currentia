// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PROCESSOR_H_
#define CURRENTIA_QUERY_PROCESSOR_H_

#include "currentia/core/thread.h"
#include "currentia/core/scheduler/abstract-scheduler.h"

namespace currentia {
    class QueryProcessor : public thread::Runnable {
        AbstractScheduler* scheduler_;

    public:
        QueryProcessor(AbstractScheduler* scheduler_) {
        }

        void run() {
            try {
                while (true) {
                    scheduler_->wake_up();
                    thread::scheduler_yield();
                }
            } catch (const char* error_message) {
                std::cerr << "Error while processing stream: " << error_message << std::endl;
            }
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_PROCESSOR_H_ */
