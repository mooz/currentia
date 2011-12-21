// -*- c++ -*-

#ifndef CURRENTIA_CONDITION_H__
#define CURRENTIA_CONDITION_H__

#include "currentia/core/pointer.h"
#include "currentia/core/object.h"
#include "currentia/core/tuple.h"
#include "currentia/core/operator/comparator.h"

#include <list>

namespace currentia {
    class Condition {
    public:
        typedef std::tr1::shared_ptr<Condition> ptr_t;

        // for selection
        virtual bool check(Tuple::ptr_t tuple_ptr) = 0;
        // for join
        // virtual bool check(std::list<Tuple::ptr_t> tuple_ptrs) = 0;
    };

    class ConditionConjunctive: public Condition {
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

    public:
        ConditionConjunctive(Condition::ptr_t left_condition,
                             Condition::ptr_t right_condition,
                             ConditionConjunctive::Type type):
            type_(type),
            left_condition_(left_condition),
            right_condition_(right_condition) {
        }

        bool check(Tuple::ptr_t tuple) {
            switch (type_) {
            case ConditionConjunctive::AND:
                return left_condition_->check(tuple) && right_condition_->check(tuple);
            case ConditionConjunctive::OR:
                return left_condition_->check(tuple) || right_condition_->check(tuple);
            }
        }
    };

    // Comparator

    class ConditionComparator: public Condition {
        std::string target_attribute_name_;
        Comparator::Type comparator_type_;
        Object condition_value_;

    public:
        ConditionComparator(std::string target_attribute_name,
                            Comparator::Type comparator_type,
                            Object condition_value):
            target_attribute_name_(target_attribute_name),
            comparator_type_(comparator_type),
            condition_value_(condition_value) {
        }

        bool check(Tuple::ptr_t tuple_ptr) {
            Object target_value = tuple_ptr->
                                  get_value_by_attribute_name(target_attribute_name_);
            return target_value.compare(condition_value_, comparator_type_);
        }
    };
}

#endif  /* ! CURRENTIA_CONDITION_H__ */
