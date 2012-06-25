// -*- c++ -*-

#ifndef CURRENTIA_POINTER_H_
#define CURRENTIA_POINTER_H_

// TODO: more portable (do not use tr1::shared_ptr)
#include <tr1/memory>

namespace std {
    using tr1::shared_ptr;
    using tr1::dynamic_pointer_cast;
};

#endif  /* ! CURRENTIA_POINTER_H_ */
