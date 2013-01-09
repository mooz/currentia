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
                            std::string delimiter = ", ",
                            std::ostream& out = std::clog) {
            for (; iter != iter_end;) {
                out << *iter;
                if (++iter == iter_end)
                    break;
                out << delimiter;
            }
            out << std::endl;
        }
        template <typename Iterable>
        void print_iterable(const Iterable& iterable,
                            std::string delimiter = ", ",
                            std::ostream& out = std::clog) {
            print_iterable(iterable.begin(), iterable.end(), delimiter, out);
        }
        template <typename Iterator>
        void print_iterable_string(Iterator iter,
                                   Iterator iter_end,
                                   std::string delimiter = ", ",
                                   std::ostream& out = std::clog) {
            for (; iter != iter_end;) {
                out << (*iter)->toString();
                if (++iter == iter_end)
                    break;
                out << delimiter;
            }
            out << std::endl;
        }
        template <typename Iterable>
        void print_iterable_string(const Iterable& iterable,
                                   std::string delimiter = ", ",
                                   std::ostream& out = std::clog) {
            print_iterable_string(iterable.begin(), iterable.end(), delimiter, out);
        }
    }
}

#endif  /* ! CURRENTIA_PRINT_H_  */
