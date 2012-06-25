// -*- c++ -*-

#ifndef CURRENTIA_VISITABLE_OPERATOR_H_
#define CURRENTIA_VISITABLE_OPERATOR_H_

#include "currentia/core/operator/operator-abstract-visitor.h"

namespace currentia {
    template <typename T> class VisitableOperator {
    public:
        void accept(OperatorAbstractVisitor* const visitor) {
            visitor->visit(*reinterpret_cast<T*>(this));
        }
    };
}

#endif  /* ! CURRENTIA_VISITABLE_OPERATOR_H_ */
