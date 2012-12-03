// -*- c++ -*-

#ifndef CURRENTIA_SHOW_H_
#define CURRENTIA_SHOW_H_

#include <ostream>
#include <string>

namespace currentia {
    class Show {
    public:
        std::string toString() const { return "Unknown Show Type"; }
    };
}

std::ostream& operator<<(std::ostream& stream, const currentia::Show& show) {
    return stream << show.toString();
}

#endif  /* ! CURRENTIA_SHOW_H_ */
