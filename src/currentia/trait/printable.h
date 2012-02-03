// -*- c++ -*-

#ifndef CURRENTIA_PRINTABLE_H_
#define CURRENTIA_PRINTABLE_H_

#include <iostream>

namespace currentia {
    template <typename T> class Printable {
    public:
        // adds << operator

        friend std::ostream& operator<<(std::ostream& stream, const T& self) {
            return stream << self.toString();
        }
    };
}

#endif  /* ! CURRENTIA_PRINTABLE_H_ */
