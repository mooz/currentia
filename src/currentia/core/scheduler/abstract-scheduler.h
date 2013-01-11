// -*- c++ -*-

#ifndef CURRENTIA_ABSTRACT_SCHEDULER_H_
#define CURRENTIA_ABSTRACT_SCHEDULER_H_

#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/operator-visitor-serializer.h"
#include "currentia/core/scheduler/policy/scheduling-policy.h"

#include "currentia/trait/pointable.h"

namespace currentia {
    class AbstractScheduler : public Pointable<AbstractScheduler> {
    private:
        int batch_count_;       // (maximum) number of tuples to be evaluated at a chance
        bool efficient_scheduling_enabled_;

    protected:
        const Operator::ptr_t root_operator_;
        const std::vector<Operator*> operators_;
        const SchedulingPolicy::ptr_t scheduling_policy_;

    public:
        AbstractScheduler(const Operator::ptr_t& root_operator,
                          const SchedulingPolicyFactory::ptr_t& scheduling_policy_factory):
            batch_count_(1),
            efficient_scheduling_enabled_(false),
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

        void set_efficient_scheduling_enabled(bool enabled) {
            efficient_scheduling_enabled_ = enabled;
        }

        bool efficient_scheduling_enabled() {
            return efficient_scheduling_enabled_;
        }

        virtual ~AbstractScheduler() {};
        virtual bool wake_up() = 0;

    protected:
        Operator* get_next_operator_() {
            return scheduling_policy_->get_next_operator();
        }

        bool process_operator_batch_(Operator* next_operator) {
            if (!next_operator)
                return false;

            if (efficient_scheduling_enabled_ &&
                next_operator->is_redo_area_leaf()) {
                next_operator->process_next(1);
            } else {
                next_operator->process_next(batch_count_);
            }

            return true;
        }
    };
}

#endif  /* ! CURRENTIA_ABSTRACT_SCHEDULER_H_ */
