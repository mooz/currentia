// -*- c++ -*-

#ifndef CURRENTIA_CONDITION_H_
#define CURRENTIA_CONDITION_H_

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
        virtual bool check(Tuple::ptr_t tuple_ptr) const = 0;
        // for join
        virtual bool check(Tuple::ptr_t tuple1_ptr, Tuple::ptr_t tuple2_ptr) const = 0;
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

        bool check(Tuple::ptr_t tuple_ptr) const {
            switch (type_) {
            case ConditionConjunctive::AND:
                return left_condition_->check(tuple_ptr)
                    && right_condition_->check(tuple_ptr);
            case ConditionConjunctive::OR:
                return left_condition_->check(tuple_ptr)
                    || right_condition_->check(tuple_ptr);
            }
        }

        bool check(Tuple::ptr_t tuple1_ptr, Tuple::ptr_t tuple2_ptr) const {
            switch (type_) {
            case ConditionConjunctive::AND:
                return left_condition_->check(tuple1_ptr)
                    && right_condition_->check(tuple2_ptr);
            case ConditionConjunctive::OR:
                return left_condition_->check(tuple1_ptr)
                    || right_condition_->check(tuple2_ptr);
            }
        }
    };

    // Comparator

    class ConditionConstantComparator: public Condition {
        std::string target_attribute_name_;
        Comparator::Type comparator_type_;
        Object condition_value_;

    public:
        ConditionConstantComparator(std::string target_attribute_name,
                                    Comparator::Type comparator_type,
                                    Object condition_value):
            target_attribute_name_(target_attribute_name),
            comparator_type_(comparator_type),
            condition_value_(condition_value) {
        }

        bool check(Tuple::ptr_t tuple_ptr) const {
            Object target_value = tuple_ptr->
                                  get_value_by_attribute_name(target_attribute_name_);
            return target_value.compare(condition_value_, comparator_type_);
        }

        bool check(Tuple::ptr_t tuple1_ptr, Tuple::ptr_t tuple2_ptr) const {
            throw "Error: ConditionConstantComparator doesn't support comparison of 2 tuples";
        }
    };

    class ConditionAttributeComparator: public Condition {
        std::string target1_attribute_name_;
        std::string target2_attribute_name_;
        Comparator::Type comparator_type_;

    public:
        ConditionAttributeComparator(std::string target1_attribute_name,
                                     Comparator::Type comparator_type,
                                     std::string target2_attribute_name):
            target1_attribute_name_(target1_attribute_name),
            target2_attribute_name_(target2_attribute_name),
            comparator_type_(comparator_type) {
        }

        bool check(Tuple::ptr_t tuple_ptr) const {
            throw "Error: ConditionConstantComparator doesn't support comparison of tuple and constant";
        }

        bool check(Tuple::ptr_t tuple1_ptr, Tuple::ptr_t tuple2_ptr) const {
            Object tuple1_value = tuple1_ptr->
                                  get_value_by_attribute_name(target1_attribute_name_);
            Object tuple2_value = tuple2_ptr->
                                  get_value_by_attribute_name(target2_attribute_name_);
            return tuple1_value.compare(tuple2_value, comparator_type_);
        }
    };
}

#endif  /* ! CURRENTIA_CONDITION_H_ */
