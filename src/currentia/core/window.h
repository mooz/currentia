// -*- c++ -*-

#ifndef CURRENTIA_WINDOW_H_
#define CURRENTIA_WINDOW_H_

namespace currentia {
    class Window {
    public:
        enum Type {
            LOGICAL,
            PHYSICAL
        };

        long width;
        long stride;
        Window::Type type;

        Window(long width, long stride, Window::Type type = LOGICAL):
            width(width),
            stride(stride),
            type(type) {
            if (stride < 1)
                stride = 1;
        }
    };
}

#endif  /* ! CURRENTIA_WINDOW_H_ */
