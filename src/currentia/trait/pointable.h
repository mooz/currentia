// -*- c++ -*-

#ifndef CURRENTIA_POINTABLE_H_
#define CURRENTIA_POINTABLE_H_

#include "currentia/core/pointer.h"

namespace currentia {
    template <typename T> class Pointable {
    public:
        typedef std::shared_ptr<T> ptr_t;
    };
}

#endif  /* ! CURRENTIA_POINTABLE_H_ */
