// -*- c++ -*-

#ifndef CURRENTIA_PRINT_H_
#define CURRENTIA_PRINT_H_

#include <iostream>
#include <string>

namespace currentia {
    namespace util {
        template <typename Iterator>
        void print_iterable(Iterator iter,
                            Iterator iter_end,
                            std::string delimiter = ", ") {
            for (; iter != iter_end; ++iter) {
                std::cout << *iter << delimiter;
            }
            std::cout << std::endl;
        }
    }
}

#endif  /* ! CURRENTIA_PRINT_H_  */
