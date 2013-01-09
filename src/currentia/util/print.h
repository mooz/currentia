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
            for (; iter != iter_end;) {
                std::clog << *iter;
                if (++iter == iter_end)
                    break;
                std::clog << delimiter;
            }
            std::clog << std::endl;
        }
        template <typename Iterable>
        void print_iterable(const Iterable& iterable,
                            std::string delimiter = ", ") {
            print_iterable(iterable.begin(), iterable.end(), delimiter);
        }
        template <typename Iterator>
        void print_iterable_string(Iterator iter,
                                   Iterator iter_end,
                                   std::string delimiter = ", ") {
            for (; iter != iter_end;) {
                std::clog << (*iter)->toString();
                if (++iter == iter_end)
                    break;
                std::clog << delimiter;
            }
            std::clog << std::endl;
        }
        template <typename Iterable>
        void print_iterable_string(const Iterable& iterable,
                                   std::string delimiter = ", ") {
            print_iterable_string(iterable.begin(), iterable.end(), delimiter);
        }
    }
}

#endif  /* ! CURRENTIA_PRINT_H_  */
