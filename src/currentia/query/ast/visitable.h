// -*- c++ -*-

#ifndef CURRENTIA_VISITABLE_H_
#define CURRENTIA_VISITABLE_H_

// #include <iostream>

#include "currentia/query/ast/abstract-visitor.h"

namespace currentia {
    template <typename T> class Visitable {
    public:
        void accept(AbstractVisitor* const visitor) {
            visitor->visit(*reinterpret_cast<T*>(this));
        }
    };
}

#endif  /* ! CURRENTIA_VISITABLE_H_ */
