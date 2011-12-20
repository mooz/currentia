// -*- c++ -*-

#ifndef CURRENTIA_CONDITION_H__
#define CURRENTIA_CONDITION_H__

#include "currentia/core/pointer.h"
#include "currentia/core/object.h"
#include "currentia/core/operator/comparator.h"

namespace currentia {
    class Condition {
    public:
        typedef std::tr1::shared_ptr<Condition> ptr_t;

        virtual bool eval() = 0;
    };

    class ConditionConjunctive {
    public:
        // Conjunctive
        enum Type {
            AND,
            OR
        };

    private:
        enum ConditionConjunctive::Type type_;

        Condition::ptr_t left_condition_;
        Condition::ptr_t right_condition_;

        ConditionConjunctive::Type comparator_;

    public:
        ConditionConjunctive(Condition::ptr_t left_condition,
                             Condition::ptr_t right_condition,
                             ConditionConjunctive::Type type):
            type_(type),
            left_condition_(left_condition),
            right_condition_(right_condition) {
        }

        bool eval() {
            switch (type_) {
            case ConditionConjunctive::AND:
                return left_condition_->eval() && right_condition_->eval();
            case ConditionConjunctive::OR:
                return left_condition_->eval() || right_condition_->eval();
            }
        }
    };

    // Comparator

    class ConditionComparator {
        Comparator::Type type_;
        Object left_object_;
        Object right_object_;

    public:
        ConditionComparator(Comparator::Type type,
                            Object left_object,
                            Object right_object):
            type_(type),
            left_object_(left_object),
            right_object_(right_object) {
        }

        bool eval() {
            return left_object_.compare(right_object_, type_);
        }
    };
}

#endif  /* ! CURRENTIA_CONDITION_H__ */
