// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_ABSTRACT_VISITOR_H_
#define CURRENTIA_OPERATOR_ABSTRACT_VISITOR_H_

#include "currentia/trait/pointable.h"
#include "currentia/core/operator/operator-abstract-visitor.h"
#include "currentia/core/operator/operator-forward-declaration.h"
// #include "currentia/core/operator/operator.h"

namespace currentia {
    CURRENTIA_OPERATOR_LIST(CURRENTIA_OPERATOR_DECLARE)

    class OperatorAbstractVisitor : public Pointable<OperatorAbstractVisitor> {
    public:
        typedef Pointable<OperatorAbstractVisitor>::ptr_t ptr_t;

        OperatorAbstractVisitor() {
        }

        virtual ~OperatorAbstractVisitor() = 0;

        CURRENTIA_OPERATOR_LIST(CURRENTIA_OPERATOR_DEFINE_VISIT)
    };

    OperatorAbstractVisitor::~OperatorAbstractVisitor() {}
}




#endif  /* ! CURRENTIA_OPERATOR_ABSTRACT_VISITOR_H_ */
