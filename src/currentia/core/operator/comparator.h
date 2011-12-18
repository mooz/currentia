// -*- c++ -*-

#ifndef CURRENTIA_COMPARATOR_H__
#define CURRENTIA_COMPARATOR_H__

namespace currentia {
    enum ComparatorType {
        COMPARATOR_EQUAL,              // ==
        COMPARATOR_NOT_EQUAL,          // !=
        COMPARATOR_LESS_THAN,          // <
        COMPARATOR_LESS_THAN_EQUAL,    // <=
        COMPARATOR_GREATER_THAN,       // >
        COMPARATOR_GREATER_THAN_EQUAL, // >=
    };
}

#endif  /* ! CURRENTIA_COMPARATOR_H__ */
