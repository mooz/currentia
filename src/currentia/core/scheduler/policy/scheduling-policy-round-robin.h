// -*- c++ -*-

#ifndef CURRENTIA_SCHEDULING_POLICY_ROUND_ROBIN_H_
#define CURRENTIA_SCHEDULING_POLICY_ROUND_ROBIN_H_

#include "currentia/core/operator/operator.h"
#include "./scheduling-policy.h"

namespace currentia {
    class RoundRobinPolicy : public SchedulingPolicy {
        int current_operator_index_;

    public:
        RoundRobinPolicy(const Operator::ptr_t& root_operator):
            SchedulingPolicy(root_operator),
            current_operator_index_(0) {
        }

        Operator* get_next_operator() {
            // TODO: Guarantee the returned operator is qualified (e.g., has input events)?
            int current_index = current_operator_index_;
            current_operator_index_ = (current_operator_index_ + 1) % number_of_operators_;
            return operators_[current_index];
        }

        void reset() {
            current_operator_index_ = 0;
        }
    };

    struct RoundRobinPolicyFactory : public SchedulingPolicyFactory {
        SchedulingPolicy*
        create_policy_in_heap(const Operator::ptr_t& root_operator) const {
            return new RoundRobinPolicy(root_operator);
        }
    };
}

#endif  /* ! CURRENTIA_SCHEDULING_POLICY_ROUND_ROBIN_H_ */
