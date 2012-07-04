// -*- c++ -*-

#ifndef CURRENTIA_SHOW_H_
#define CURRENTIA_SHOW_H_

#include <ostream>
#include <string>

namespace currentia {
    class Show {
    public:
        virtual ~Show() = 0;
        virtual std::string toString() const = 0;
    };
    Show::~Show() {}
}

std::ostream& operator<<(std::ostream& stream, const currentia::Show& show) {
    return stream << show.toString();
}

#endif  /* ! CURRENTIA_SHOW_H_ */
