// -*- c++ -*-

#ifndef CURRENTIA_ABSTRACT_SCHEDULER_H_
#define CURRENTIA_ABSTRACT_SCHEDULER_H_

#include "currentia/core/operator/operator.h"

namespace currentia {
    class AbstractScheduler {
    public:
        AbstractScheduler(const Operator::ptr_t& root_operator) {
        }

        virtual ~AbstractScheduler() = 0;
        virtual void wake_up() = 0;
    };

    AbstractScheduler::~AbstractScheduler() {};
}

#endif  /* ! CURRENTIA_ABSTRACT_SCHEDULER_H_ */
