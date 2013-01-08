// -*- c++ -*-

#ifndef CURRENTIA_ABSTRACT_SCHEDULER_H_
#define CURRENTIA_ABSTRACT_SCHEDULER_H_

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/operator-visitor-serializer.h"
#include "currentia/core/scheduler/policy/scheduling-policy.h"

namespace currentia {
    class AbstractScheduler {
    public:
        int batch_count_;       // (maximum) number of tuples to be evaluated at a chance

    protected:
        const Operator::ptr_t root_operator_;
        const std::vector<Operator*> operators_;
        const SchedulingPolicy::ptr_t scheduling_policy_;

    public:
        AbstractScheduler(const Operator::ptr_t& root_operator,
                          const SchedulingPolicyFactory::ptr_t& scheduling_policy_factory):
            batch_count_(1),
            root_operator_(root_operator),
            operators_(OperatorVisitorSerializer::serialize_tree(root_operator.get())),
            // To make scheduler free its policy in destruction phase,
            // wrap policy with shared_ptr
            scheduling_policy_(SchedulingPolicy::ptr_t(scheduling_policy_factory->create_policy_in_heap(root_operator))) {
        }

        int batch_count() const {
            return batch_count_;
        }

        void set_batch_count(int batch_count) {
            batch_count_ = batch_count;
        }

        virtual ~AbstractScheduler() = 0;
        virtual void wake_up() = 0;
    };

    AbstractScheduler::~AbstractScheduler() {};
}

#endif  /* ! CURRENTIA_ABSTRACT_SCHEDULER_H_ */
