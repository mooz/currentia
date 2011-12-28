// -*- c++ -*-

#ifndef CURRENTIA_SYNOPSIS_H_
#define CURRENTIA_SYNOPSIS_H_

#include <vector>

#include "currentia/core/tuple.h"
#include "currentia/core/window.h"
#include "currentia/core/operator/operator.h"

namespace currentia {
    class Synopsis {
    public:
        typedef std::vector<Tuple::ptr_t> tuples_t;
        typedef tuples_t::const_iterator const_iterator;

        Synopsis(Window &window):
            window_(window),
            index_(0) {
        }

        // read tuples to prepare for next join
        void read_next_tuples(Operator::ptr_t& target_operator) {
            switch (window_.type) {
            case Window::LOGICAL:
                return read_next_tuples_logical(target_operator);
            case Window::PHYSICAL:
                // TODO: support physical window (is it possible in pull-style processing?)
                return read_next_tuples_logical(target_operator);
            }
        }

        inline void read_next_tuples_logical(Operator::ptr_t& target_operator) {
            long tuples_count = tuples_.size();
            long read_count;

            if (tuples_count != window_.width) {
                // this is the first time
                read_count = window_.width;
                tuples_.resize(window_.width);
            } else {
                read_count = window_.stride; // read a tuple ${stride of sliding window} times
            }

            for (int i = 0; i < read_count; ++i) {
                tuples_[get_next_index_()] = target_operator->next();
            }
        }

        inline void read_next_tuples_physical(Operator::ptr_t& target_operator) {
            // not implemented yet
        }

        inline tuples_t::const_iterator begin() {
            return tuples_.begin();
        }

        inline tuples_t::const_iterator end() {
            return tuples_.end();
        }

        std::string toString() {
            std::stringstream ss;
            tuples_t::iterator iter = tuples_.begin();

            int i = 0;
            for (; iter < tuples_.end(); ++iter) {
                ss << "[" << i++ << "] " << (*iter)->toString() << std::endl;
            }

            return ss.str();
        }

        inline long get_next_index_() {
            return index_++ % window_.width;
        }

    private:
        tuples_t tuples_;
        Window window_;
        long index_;
    };
}

#endif  /* ! CURRENTIA_SYNOPSIS_H_ */
