// -*- c++ -*-

#ifndef CURRENTIA_QUERY_AST_ABSTRACT_VISITOR_H_
#define CURRENTIA_QUERY_AST_ABSTRACT_VISITOR_H_

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"
#include "currentia/core/operator/operator-forward-declaration.h"

namespace currentia {
    CURRENTIA_OPERATOR_LIST(CURRENTIA_OPERATOR_DECLARE)

    class AbstractVisitor : private NonCopyable<AbstractVisitor>,
                            public Pointable<AbstractVisitor> {

    public:
        virtual ~AbstractVisitor() = 0;

        CURRENTIA_OPERATOR_LIST(CURRENTIA_OPERATOR_DEFINE_VISIT)
    };

    AbstractVisitor::~AbstractVisitor() {}
}

#endif  /* ! CURRENTIA_QUERY_AST_ABSTRACT_VISITOR_H_ */
