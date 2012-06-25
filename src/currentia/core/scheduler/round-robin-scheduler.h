// -*- c++ -*-

#ifndef CURRENTIA_ROUND_ROBIN_SCHEDULER_H_
#define CURRENTIA_ROUND_ROBIN_SCHEDULER_H_

#include "currentia/core/scheduler/abstract-scheduler.h"
#include "currentia/core/operator/operator-visitor-serializer.h"

namespace currentia {
    class RoundRobinScheduler : public AbstractScheduler {
        OperatorVisitorSerializer serializer_;
        std::vector<Operator*> operators_;
        int current_operator_index_;

    public:
        RoundRobinScheduler(Operator::ptr_t root_operator):
            AbstractScheduler(root_operator),
            current_operator_index_(0) {
            serializer_.dispatch(root_operator.get());
            operators_ = serializer_.get_sorted_operators();
        }

        void process_next() {
            Operator* next_operator = get_next_operator_();
            next_operator->next();
        }

    private:
        Operator* get_next_operator_() {
            return operators_[get_next_operator_index_()];
        }

        int get_next_operator_index_() {
            return current_operator_index_++ % operators_.size();
        }
    };
}

#endif  /* ! CURRENTIA_ROUND_ROBIN_SCHEDULER_H_ */
