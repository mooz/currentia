// -*- c++ -*-

#ifndef CURRENTIA_POINTER_H_
#define CURRENTIA_POINTER_H_


#if defined(__GXX_EXPERIMENTAL_CXX0X__)
# include <memory>
#else
# include <tr1/memory>
namespace std {
    using tr1::shared_ptr;
    using tr1::dynamic_pointer_cast;
};
#endif

#endif  /* ! CURRENTIA_POINTER_H_ */
