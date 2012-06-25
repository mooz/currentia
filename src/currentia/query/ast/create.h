// -*- c++ -*-

#ifndef CURRENTIA_QUERY_AST_CREATE_H_
#define CURRENTIA_QUERY_AST_CREATE_H_

#include <list>

#include "currentia/query/ast/statement.h"
#include "currentia/query/ast/visitable.h"

#include "currentia/core/attribute.h"

// Traits
#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    struct Create: public Statement,
                   public Pointable<Create>,
                   public Visitable<Create> {

        typedef Pointable<Create>::ptr_t ptr_t;

        // name
        std::string name_;
        // attributes
        // std::list<Attribute> attributes;

        std::string get_name() {
            return name_;
        }

        bool has_children() {
            return !is_leaf_node_;
        }

        // @Formal
        // Create(const std::string& name, std::list<Attribute> attributes):
        //     name(name),
        //     attributes(attributes) {
        // }

        Create::ptr_t left_;
        Create::ptr_t right_;
        bool is_leaf_node_;

        Create(const std::string name):
            name_(name),
            left_(Create::ptr_t()),
            right_(Create::ptr_t()),
            is_leaf_node_(true) {
        }

        Create(const std::string name,
               const Create::ptr_t& left, const Create::ptr_t& right):
            name_(name),
            left_(left),
            right_(right),
            is_leaf_node_(false) {
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_AST_CREATE_H_ */
