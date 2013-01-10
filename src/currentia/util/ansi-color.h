// -*- c++ -*-

#ifndef CURRENTIA_ANSI_H_
#define CURRENTIA_ANSI_H_

#include <iostream>
#include <string>

namespace currentia {
    namespace ansi {
        // enum Colors {
        //     reset        = 0,
        //     bold         = 1,
        //     bright       = 1,
        //     dim          = 2,
        //     underline    = 4,
        //     underscore   = 4,
        //     blink        = 5,
        //     reverse      = 7,
        //     hidden       = 8,
        //     black        = 30,
        //     red          = 31,
        //     green        = 32,
        //     yellow       = 33,
        //     blue         = 34,
        //     magenta      = 35,
        //     cyan         = 36,
        //     white        = 37,
        //     on_black     = 40,
        //     on_red       = 41,
        //     on_green     = 42,
        //     on_yellow    = 43,
        //     on_blue      = 44,
        //     on_magenta   = 45,
        //     on_cyan      = 46,
        //     on_white     = 47,
        // };

        const char* reset        = "\033[0m";
        const char* bold         = "\033[1m";
        const char* bright       = "\033[1m";
        const char* dim          = "\033[2m";
        const char* underline    = "\033[4m";
        const char* underscore   = "\033[4m";
        const char* blink        = "\033[5m";
        const char* reverse      = "\033[7m";
        const char* hidden       = "\033[8m";
        const char* black        = "\033[30m";
        const char* red          = "\033[31m";
        const char* green        = "\033[32m";
        const char* yellow       = "\033[33m";
        const char* blue         = "\033[34m";
        const char* magenta      = "\033[35m";
        const char* cyan         = "\033[36m";
        const char* white        = "\033[37m";
        const char* on_black     = "\033[40m";
        const char* on_red       = "\033[41m";
        const char* on_green     = "\033[42m";
        const char* on_yellow    = "\033[43m";
        const char* on_blue      = "\033[44m";
        const char* on_magenta   = "\033[45m";
        const char* on_cyan      = "\033[46m";
        const char* on_white     = "\033[47m";

        std::ostream&
        colored(std::ostream& output_stream) {
            return output_stream << reset;
        }
    }
}

#endif  /* ! CURRENTIA_ANSI_H_  */
