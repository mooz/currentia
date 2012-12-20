// -*- c++ -*-

#ifndef CURRENTIA_WITHOUT_CC_SCHEDULER_H_
#define CURRENTIA_WITHOUT_CC_SCHEDULER_H_

#include "currentia/core/cc/abstract-cc-scheduler.h"

namespace currentia {
    class WithoutCCScheduler : public AbstractCCScheduler {
    public:
        WithoutCCScheduler(const Operator::ptr_t& root_operator,
                           const SchedulingPolicyFactory::ptr_t& scheduling_policy_factory):
            AbstractCCScheduler(root_operator,
                                scheduling_policy_factory,
                                Operator::NONE) {
        }

        void wake_up() {
            Operator* next_operator = get_next_operator_();
            next_operator->process_next();
        }
    };
}

#endif  /* ! CURRENTIA_WITHOUT_CC_SCHEDULER_H_ */
