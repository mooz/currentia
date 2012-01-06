// -*- c++ -*-

#ifndef CURRENTIA_COMPARATOR_H_
#define CURRENTIA_COMPARATOR_H_

#include <string>

namespace currentia {
    namespace Comparator {
        enum Type {
            EQUAL,              // ==
            NOT_EQUAL,          // !=
            LESS_THAN,          // <
            LESS_THAN_EQUAL,    // <=
            GREATER_THAN,       // >
            GREATER_THAN_EQUAL, // >=
        };

        std::string comparator_to_string(Type type) {
            switch (type) {
            case EQUAL:
                return "==";
            case NOT_EQUAL:
                return "!=";
            case LESS_THAN:
                return "<";
            case LESS_THAN_EQUAL:
                return "<=";
            case GREATER_THAN:
                return ">";
            case GREATER_THAN_EQUAL:
                return ">=";
            }
        }
    }
}

#endif  /* ! CURRENTIA_COMPARATOR_H_ */
