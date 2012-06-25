// -*- c++ -*-

#ifndef CURRENTIA_QUERY_AST_FROM_SELECTION_H_
#define CURRENTIA_QUERY_AST_FROM_SELECTION_H_

#include <list>
#include <string>

// AST Nodes
#include "currentia/query/ast/statement.h"

// DSMS
#include "currentia/core/window.h"

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

// #include "currentia/core/pointer.h"

// #include "currentia/core/operator/operator.h"

namespace currentia {
    struct SelectionNode : public Statement {
    public:
    };

    struct FromSelectionNode : public SelectionNode {
        // selected attributes
        std::list<SelectionNode::ptr_t> attribute_names_;
        void set_attribute_names(const std::list<SelectionNode::ptr_t>& attribute_names) {
            attribute_names_ = attribute_names;
        }

        // window information
        Window window_;
        // from selection nodes
        std::list<SelectionNode::ptr_t> from_nodes_;

    public:
        FromSelectionNode(Window window) : window_(window) {
        }
    };

    struct FromStreamNode : public SelectionNode {
        // selected attributes
        std::string stream_name;

        // window information
        Window window_;

        // from nodes
        std::list<SelectionNode::ptr_t> from_nodes_;
    };

    // ------------------------------------------------------------ //
    // DDL
    // ------------------------------------------------------------ //

    struct CreateNode : public Statement,
                        public Pointable<CreateNode> {
        typedef Pointable<CreateNode>::ptr_t ptr_t;

        // name
        std::string name;
        // attributes
        std::list<Attribute> attributes;

        CreateNode(const std::string& name, std::list<Attribute> attributes) :
            name(name),
            attributes(attributes) {
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_AST_FROM_SELECTION_H_ */
