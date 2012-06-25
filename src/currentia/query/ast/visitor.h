// -*- c++ -*-

#ifndef CURRENTIA_QUERY_AST_VISITOR_H_
#define CURRENTIA_QUERY_AST_VISITOR_H_

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"
#include "currentia/query/ast/abstract-visitor.h"

#include "currentia/query/ast/create.h"

namespace currentia {
    class Visitor : public AbstractVisitor,
                    private NonCopyable<Visitor>,
                    public Pointable<Visitor> {
    public:
        void visit(Create& create_node) {
            std::cerr << "Visit create node : "
                      << create_node.get_name() << std::endl;
            if (create_node.has_children()) {
                create_node.left_->accept(this);
                create_node.right_->accept(this);
            }
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_AST_VISITOR_H_ */
