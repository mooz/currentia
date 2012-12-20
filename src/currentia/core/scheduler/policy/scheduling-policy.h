// -*- c++ -*-

#ifndef CURRENTIA_SCHEDULING_POLICY_H_
#define CURRENTIA_SCHEDULING_POLICY_H_

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/operator-visitor-serializer.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class SchedulingPolicy : public Pointable<SchedulingPolicy> {
    protected:
        Operator::ptr_t root_operator_;
        const std::vector<Operator*> operators_;
        int number_of_operators_;

    public:
        SchedulingPolicy(const Operator::ptr_t& root_operator):
            root_operator_(root_operator),
            operators_(OperatorVisitorSerializer::serialize_tree(root_operator)),
            number_of_operators_(operators_.size()) {
        }

        virtual Operator* get_next_operator() = 0;
        virtual ~SchedulingPolicy() = 0;
        virtual void reset() {}
    };
    SchedulingPolicy::~SchedulingPolicy() {};

    // We should arrange a factory class per a policy
    class SchedulingPolicyFactory : public Pointable<SchedulingPolicyFactory> {
    public:
        SchedulingPolicyFactory() {}

        virtual SchedulingPolicy*
        create_policy_in_heap(const Operator::ptr_t& root_operator) const = 0;

        virtual ~SchedulingPolicyFactory() = 0;
    };
    SchedulingPolicyFactory::~SchedulingPolicyFactory() {};
}

#endif  /* ! CURRENTIA_SCHEDULING_POLICY_H_ */
