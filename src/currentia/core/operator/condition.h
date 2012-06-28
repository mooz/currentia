// -*- c++ -*-

#ifndef CURRENTIA_CONDITION_H_
#define CURRENTIA_CONDITION_H_

#include "currentia/core/pointer.h"
#include "currentia/core/object.h"
#include "currentia/core/tuple.h"
#include "currentia/core/operator/comparator.h"
#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

#include <list>

namespace currentia {
    class Condition: private NonCopyable<Condition>,
                     public Pointable<Condition> {
    protected:
        bool negated_;

    public:
        Condition(): negated_(false) {}
        virtual ~Condition() = 0;

        // for selection
        virtual bool check(Tuple::ptr_t tuple_ptr) const = 0;
        // for join
        virtual bool check(Tuple::ptr_t left_tuple_ptr, Tuple::ptr_t right_tuple_ptr) const = 0;

        Condition* negate() {
            negated_ = !negated_;
            return this;
        }

        virtual void obey_schema(const Schema::ptr_t& left_schema,
                                 const Schema::ptr_t& right_schema) = 0;
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

    class ConditionConjunctive: public Condition,
                                public Pointable<ConditionConjunctive> {
    public:
        typedef Pointable<ConditionConjunctive>::ptr_t ptr_t;

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

        bool check(Tuple::ptr_t left_tuple_ptr, Tuple::ptr_t right_tuple_ptr) const {
            switch (type_) {
            case ConditionConjunctive::AND:
                return left_condition_->check(left_tuple_ptr, right_tuple_ptr)
                    && right_condition_->check(left_tuple_ptr, right_tuple_ptr);
            case ConditionConjunctive::OR:
                return left_condition_->check(left_tuple_ptr, right_tuple_ptr)
                    || right_condition_->check(left_tuple_ptr, right_tuple_ptr);
            }
        }

        void obey_schema(const Schema::ptr_t& left_schema,
                         const Schema::ptr_t& right_schema) {
            left_condition_->obey_schema(left_schema, right_schema);
            right_condition_->obey_schema(left_schema, right_schema);
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
            // bottom-up
            left_condition_->distribute_disjunction();
            right_condition_->distribute_disjunction();

            if (type_ != OR)
                return;

            ConditionConjunctive::ptr_t left_conjunctive =
                std::dynamic_pointer_cast<ConditionConjunctive>(left_condition_);
            ConditionConjunctive::ptr_t right_conjunctive =
                std::dynamic_pointer_cast<ConditionConjunctive>(right_condition_);

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
            ConditionConjunctive::ptr_t target_conjunctive =
                std::dynamic_pointer_cast<ConditionConjunctive>(target_condition);

            bool result = target_conjunctive &&
                          left_condition_->equal_to(target_conjunctive->get_left_condition()) &&
                          right_condition_->equal_to(target_conjunctive->get_right_condition());

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

    class ConditionConstantComparator: public Condition,
                                       public Pointable<ConditionConstantComparator> {
    public:
        typedef Pointable<ConditionConstantComparator>::ptr_t ptr_t;

    private:

        std::string target_attribute_name_;
        Comparator::Type comparator_type_;
        Object condition_value_;
        bool target_tuple_is_left_;

    public:
        ConditionConstantComparator(std::string target_attribute_name,
                                    Comparator::Type comparator_type,
                                    Object condition_value):
            target_attribute_name_(target_attribute_name),
            comparator_type_(comparator_type),
            condition_value_(condition_value),
            target_tuple_is_left_(true) {
        }

        bool check(Tuple::ptr_t tuple_ptr) const {
            Object target_value = tuple_ptr->
                                  get_value_by_attribute_name(target_attribute_name_);
            return target_value.compare(condition_value_, comparator_type_);
        }

        bool check(Tuple::ptr_t left_tuple_ptr, Tuple::ptr_t right_tuple_ptr) const {
            if (target_tuple_is_left_)
                return check(left_tuple_ptr);
            else
                return check(right_tuple_ptr);
        }

        void obey_schema(const Schema::ptr_t& left_schema,
                         const Schema::ptr_t& right_schema) {
            if (left_schema->has_attribute(target_attribute_name_)) {
                target_tuple_is_left_ = true;
            } else if (left_schema->has_attribute(target_attribute_name_)) {
                target_tuple_is_left_ = false;
            } else {
                std::stringstream ss;
                ss << "Attribute in (" << this->toString()
                   << ") is missing in schemas " << left_schema->toString()
                   << " and " << right_schema->toString();
            }
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
            ConditionConstantComparator::ptr_t target_constant_comparator =
                std::dynamic_pointer_cast<ConditionConstantComparator>(target_condition);

            bool result = target_constant_comparator &&
                          target_attribute_name_ == target_constant_comparator->get_target_attribute_name() &&
                          comparator_type_ == target_constant_comparator->get_target_comparator_type() &&
                          condition_value_ == target_constant_comparator->get_condition_value();

            return result;
        }
    };

    class ConditionAttributeComparator: public Condition,
                                        public Pointable<ConditionAttributeComparator> {
        std::string left_attribute_name_;
        std::string right_attribute_name_;
        Comparator::Type comparator_type_;

    public:
        typedef Pointable<ConditionAttributeComparator>::ptr_t ptr_t;

        ConditionAttributeComparator(std::string left_attribute_name,
                                     Comparator::Type comparator_type,
                                     std::string right_attribute_name):
            left_attribute_name_(left_attribute_name),
            right_attribute_name_(right_attribute_name),
            comparator_type_(comparator_type) {
        }

        bool check(Tuple::ptr_t tuple_ptr) const {
            std::stringstream ss;
            ss << "ConditionAttributeComparator doesn't support comparison of tuple and constant";
            throw ss.str();
        }

        bool check(Tuple::ptr_t left_tuple_ptr, Tuple::ptr_t right_tuple_ptr) const {
            Object left_tuple_value = left_tuple_ptr->
                                      get_value_by_attribute_name(left_attribute_name_);
            Object right_tuple_value = right_tuple_ptr->
                                       get_value_by_attribute_name(right_attribute_name_);
            return left_tuple_value.compare(right_tuple_value, comparator_type_);
        }

        // Called once (by in operator initialization)
        void obey_schema(const Schema::ptr_t& left_schema,
                         const Schema::ptr_t& right_schema) {
            if (left_schema->has_attribute(left_attribute_name_) &&
                right_schema->has_attribute(right_attribute_name_)) {
                // as is
                return;
            }

            if (left_schema->has_attribute(right_attribute_name_) &&
                right_schema->has_attribute(left_attribute_name_)) {
                // swap
                std::string saved_left_attribute = left_attribute_name_;
                left_attribute_name_ = right_attribute_name_;
                right_attribute_name_ = saved_left_attribute;
                return;
            }

            // Schemas don't have required attribute value.
            std::stringstream ss;
            ss << "Attribute(s) in (" << this->toString()
               << ") are missing in schemas " << left_schema->toString()
               << " or " << right_schema->toString();
            throw ss.str();
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
            return left_attribute_name_ + " " +
                comparator_to_string(comparator_type_) + " " +
                right_attribute_name_;
        }

        std::string get_left_attribute_name() const {
            return left_attribute_name_;
        }

        std::string get_right_attribute_name() const {
            return right_attribute_name_;
        }

        Comparator::Type get_comparator_type() const {
            return comparator_type_;
        }

        bool equal_to(const Condition::ptr_t& target_condition) const {
            ConditionAttributeComparator::ptr_t target_attribute_comparator =
                std::dynamic_pointer_cast<ConditionAttributeComparator>(target_condition);

            bool result = target_attribute_comparator &&
                          left_attribute_name_ == target_attribute_comparator->get_left_attribute_name() &&
                          right_attribute_name_ == target_attribute_comparator->get_right_attribute_name() &&
                          right_attribute_name_ == target_attribute_comparator->get_right_attribute_name() &&
                          comparator_type_ == target_attribute_comparator->get_comparator_type();

            return result;
        }
    };
}

#endif  /* ! CURRENTIA_CONDITION_H_ */
