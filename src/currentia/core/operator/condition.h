// -*- c++ -*-

#ifndef CURRENTIA_CONDITION_H_
#define CURRENTIA_CONDITION_H_

#include "currentia/core/pointer.h"
#include "currentia/core/object.h"
#include "currentia/core/tuple.h"
#include "currentia/core/operator/comparator.h"
#include "currentia/trait/non-copyable.h"

#include <list>

namespace currentia {
    class Condition: private NonCopyable<Condition> {
    protected:
        bool negated_;

    public:
        typedef std::tr1::shared_ptr<Condition> ptr_t;

        Condition(): negated_(false) {}
        virtual ~Condition() = 0;

        // for selection
        virtual bool check(Tuple::ptr_t tuple_ptr) const = 0;
        // for join
        virtual bool check(Tuple::ptr_t tuple1_ptr, Tuple::ptr_t tuple2_ptr) const = 0;

        Condition* negate() {
            negated_ = !negated_;
            return this;
        }

        virtual void de_morgen() = 0;
        virtual void shove_negation() = 0;
        virtual void distribute_disjunction() = 0;

        virtual std::string to_string_expression() const = 0;
        virtual bool equal_to(const Condition::ptr_t& target_condition) const = 0;

        std::string toString() const {
            std::string str_expression = to_string_expression();

            if (negated_)
                str_expression = "!(" + str_expression + ")";

            return str_expression;
        }

        void to_cnf() {
            shove_negation();
            distribute_disjunction();
        }
    };

    Condition::~Condition() {}

    class ConditionConjunctive: public Condition {
    public:
        typedef std::tr1::shared_ptr<ConditionConjunctive> ptr_t;

        // Conjunctive
        enum Type {
            AND,
            OR
        };

        std::string conjunctive_type_to_string(Type type) const {
            return std::string(type == AND ? " && " : " || ");
        }

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

        void de_morgen() {
            negate();
            type_ = type_ == AND ? OR : AND;
            left_condition_->negate();
            right_condition_->negate();
        }

        void shove_negation() {
            // top-down
            if (negated_)
                de_morgen();

            left_condition_->shove_negation();
            right_condition_->shove_negation();
        }

        void distribute_disjunction() {
            using namespace std::tr1;

            // bottom-up
            left_condition_->distribute_disjunction();
            right_condition_->distribute_disjunction();

            if (type_ != OR)
                return;

            ConditionConjunctive::ptr_t left_conjunctive =
                dynamic_pointer_cast<ConditionConjunctive>(left_condition_);
            ConditionConjunctive::ptr_t right_conjunctive =
                dynamic_pointer_cast<ConditionConjunctive>(right_condition_);

            if (left_conjunctive || right_conjunctive) {
                // (Q && R) || P -> (Q || P) && (R || P)

                ConditionConjunctive::ptr_t decompose_target = ConditionConjunctive::ptr_t();
                Condition::ptr_t inject_target = Condition::ptr_t();

                if (left_conjunctive && left_conjunctive->get_type() == AND) {
                    decompose_target = left_conjunctive;
                    inject_target    = right_condition_;
                } else if (right_conjunctive && right_conjunctive->get_type() == AND) {
                    decompose_target = right_conjunctive;
                    inject_target    = left_condition_;
                } else {
                    return;
                }

                // (Q || P)
                Condition::ptr_t new_left =
                    inject_target->equal_to(decompose_target->get_left_condition())
                    ? inject_target
                    : Condition::ptr_t(
                    new ConditionConjunctive(
                        decompose_target->get_left_condition(), // Q
                        inject_target,                          // P
                        OR
                    )
                );

                // (R || P)
                Condition::ptr_t new_right =
                    inject_target->equal_to(decompose_target->get_right_condition())
                    ? inject_target
                    : Condition::ptr_t(
                    new ConditionConjunctive(
                        decompose_target->get_right_condition(), // Q
                        inject_target,                           // P
                        OR
                    )
                );

                type_ = AND;
                left_condition_ = new_left;
                right_condition_ = new_right;
            } // if (left_conjunctive && right_conjunctive)
        }

        bool equal_to(const Condition::ptr_t& target_condition) const {
            using namespace std::tr1;

            ConditionConjunctive::ptr_t target_conjunctive =
                dynamic_pointer_cast<ConditionConjunctive>(target_condition);

            bool result = target_conjunctive &&
                left_condition_->equal_to(target_conjunctive->get_left_condition()) &&
                right_condition_->equal_to(target_conjunctive->get_right_condition());

            std::cerr << "ConditionConjunctive: Comparing " << toString() << " with " << target_condition->toString() << " => " << result << std::endl;

            return result;
        }

        std::string to_string_expression() const {
            return left_condition_->toString() +
                conjunctive_type_to_string(type_) +
                right_condition_->toString();
        }

        ConditionConjunctive::Type get_type() const {
            return type_;
        }

        Condition::ptr_t get_left_condition() const {
            return left_condition_;
        }

        Condition::ptr_t get_right_condition() const {
            return left_condition_;
        }
    };

    // Comparator

    class ConditionConstantComparator: public Condition {
    public:
        typedef std::tr1::shared_ptr<ConditionConstantComparator> ptr_t;

    private:

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

        void de_morgen() {
            // nothing
        }

        void shove_negation() {
            // nothing
        }

        void distribute_disjunction() {
            // nothing
        }

        std::string to_string_expression() const {
            return target_attribute_name_ + " " +
                comparator_to_string(comparator_type_) + " " +
                condition_value_.toString();
        }

        std::string get_target_attribute_name() const {
            return target_attribute_name_;
        }

        Comparator::Type get_target_comparator_type() const {
            return comparator_type_;
        }

        Object get_condition_value() const {
            return condition_value_;
        }

        bool equal_to(const Condition::ptr_t& target_condition) const {
            using namespace std::tr1;

            ConditionConstantComparator::ptr_t target_constant_comparator =
                dynamic_pointer_cast<ConditionConstantComparator>(target_condition);

            bool result = target_constant_comparator &&
                target_attribute_name_ == target_constant_comparator->get_target_attribute_name() &&
                comparator_type_ == target_constant_comparator->get_target_comparator_type() &&
                condition_value_ == target_constant_comparator->get_condition_value();

            std::cerr << "ConditionConstantComparator: Comparing " << toString() << " with " << target_condition->toString() << " => " << result << std::endl;

            return result;
        }
    };

    class ConditionAttributeComparator: public Condition {
        std::string target1_attribute_name_;
        std::string target2_attribute_name_;
        Comparator::Type comparator_type_;

    public:
        typedef std::tr1::shared_ptr<ConditionAttributeComparator> ptr_t;

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


        void de_morgen() {
            // nothing
        }

        void shove_negation() {
            // nothing
        }

        void distribute_disjunction() {
            // nothing
        }

        std::string to_string_expression() const {
            return target1_attribute_name_ + " " +
                comparator_to_string(comparator_type_) + " " +
                target2_attribute_name_;
        }

        std::string get_target1_attribute_name() const {
            return target1_attribute_name_;
        }

        std::string get_target2_attribute_name() const {
            return target2_attribute_name_;
        }

        Comparator::Type get_comparator_type() const {
            return comparator_type_;
        }

        bool equal_to(const Condition::ptr_t& target_condition) const {
            using namespace std::tr1;

            ConditionAttributeComparator::ptr_t target_attribute_comparator =
                dynamic_pointer_cast<ConditionAttributeComparator>(target_condition);

            bool result = target_attribute_comparator &&
                target1_attribute_name_ == target_attribute_comparator->get_target1_attribute_name() &&
                target2_attribute_name_ == target_attribute_comparator->get_target2_attribute_name() &&
                target2_attribute_name_ == target_attribute_comparator->get_target2_attribute_name() &&
                comparator_type_ == target_attribute_comparator->get_comparator_type();

            std::cerr << "ConditionAttributeComparator: Comparing " << toString() << " with " << target_condition->toString() << " => " << result << std::endl;

            return result;
        }
    };
}

#endif  /* ! CURRENTIA_CONDITION_H_ */
