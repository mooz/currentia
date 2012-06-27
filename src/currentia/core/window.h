// -*- c++ -*-

#ifndef CURRENTIA_WINDOW_H_
#define CURRENTIA_WINDOW_H_

#include <string>
#include <sstream>

namespace currentia {
    struct Window {
    public:
        enum Type {
            TUPLE_BASE,
            TIME_BASE
        };

        long width;
        long stride;
        Window::Type type;

        Window(long width, long stride, Window::Type type = TUPLE_BASE):
            width(width),
            stride(stride),
            type(type) {
            if (stride < 1)
                stride = 1;
        }

        Window() {}

        std::string toString() const {
            std::stringstream ss;

            ss << "[ width " << width << " | stride " << stride << "] ";

            switch (type) {
            case TUPLE_BASE:
                ss << "(TUPLE)";
                break;
            case TIME_BASE:
                ss << "(TIME)";
                break;
            }

            return ss.str();
        }
    };
}

#endif  /* ! CURRENTIA_WINDOW_H_ */
