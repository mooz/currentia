// -*- c++ -*-

#ifndef CURRENTIA_NON_COPYABLE_H_
#define CURRENTIA_NON_COPYABLE_H_

namespace currentia {
    // http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Non-copyable_Mixin
    template <typename T> class NonCopyable {
    protected:
        NonCopyable() {}
        ~NonCopyable() {}

    private:
        NonCopyable(const NonCopyable&);
        T& operator=(const T&);
    };
}

#endif  /* ! CURRENTIA_NON_COPYABLE_H_ */
