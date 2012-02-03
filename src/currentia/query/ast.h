// -*- c++ -*-

#ifndef CURRENTIA_QUERY_AST_H_
#define CURRENTIA_QUERY_AST_H_

#include "currentia/core/pointer.h"
#include "currentia/core/window.h"
#include "currentia/core/operator/operator.h"

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    struct AbstractNode : private NonCopyable<AbstractNode>,
                          public Pointable<AbstractNode> {
        virtual ~AbstractNode() = 0;
    };
    AbstractNode::~AbstractNode() {}

    struct SelectionNode : public AbstractNode {
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

    struct CreateNode : public AbstractNode,
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

#endif  /* ! CURRENTIA_QUERY_AST_H_ */
