// -*- c++ -*-

#ifndef CURRENTIA_QUERY_AST_ABSTRACT_VISITOR_H_
#define CURRENTIA_QUERY_AST_ABSTRACT_VISITOR_H_

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"
#include "currentia/query/ast/ast-forward-declaration.h"

namespace currentia {
    // TODO: forward declarations
    CURRENTIA_QUERY_AST_NODE_LIST(CURRENTIA_QUERY_AST_OPERATION_DECLARE)

    class AbstractVisitor : private NonCopyable<AbstractVisitor>,
                            public Pointable<AbstractVisitor> {
    public:
        virtual ~AbstractVisitor() = 0;

        CURRENTIA_QUERY_AST_NODE_LIST(CURRENTIA_QUERY_AST_OPERATION_DEFINE_VISIT)
    };

    AbstractVisitor::~AbstractVisitor() {}
}

#endif  /* ! CURRENTIA_QUERY_AST_ABSTRACT_VISITOR_H_ */
