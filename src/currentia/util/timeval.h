// -*- c++ -*-

#ifndef CURRENTIA_TIMEVAL_H_
#define CURRENTIA_TIMEVAL_H_

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>

#include "currentia/util/log.h"

namespace currentia {
    namespace timeval {
        template <typename Stream>
        void output_timeval_to_stream(const struct timeval& time, Stream& stream) {
            struct tm *date;
            int year, month, day;
            int hour, minute, second, msec;

            date   = localtime(&time.tv_sec);
            year   = date->tm_year + 1900;
            month  = date->tm_mon + 1;
            day    = date->tm_mday;
            hour   = date->tm_hour;
            minute = date->tm_min;
            second = date->tm_sec;
            msec   = time.tv_usec / 1000;

            using namespace std;
            stream << year << "/"
                   << setfill('0') << setw(2) << showbase << month << "/"
                   << setfill('0') << setw(2) << showbase << day << "/"
                   << setfill('0') << setw(2) << showbase << hour << ":"
                   << setfill('0') << setw(2) << showbase << minute << ":"
                   << setfill('0') << setw(2) << showbase << second << "("
                   << setfill('0') << setw(4) << showbase << msec << ")";
        }

        std::string timeval_to_string(const struct timeval& time) {
            std::stringstream ss;
            output_timeval_to_stream(time, ss);
            return ss.str();
        }

        long timeval_difference_msec(const struct timeval& time_a,
                                     const struct timeval& time_b) {
            long difference_msec = (time_a.tv_sec - time_b.tv_sec) * 1000;
            suseconds_t difference_usec = time_a.tv_usec - time_b.tv_usec;

            if (std::abs(difference_usec) >= 1000 * 1000) {
                // positive or negative
                long surpass_msec = difference_usec / (1000 * 1000);
                difference_msec += surpass_msec;
                difference_usec += 1000 * 1000 * surpass_msec;
            }

            return difference_msec + (difference_usec / 1000);
        }

        void timeval_add_msec(struct timeval* time, long msec) {
            // Normalize
            useconds_t add_usec = 1000 * msec;
            time_t add_sec = add_usec / (1000 * 1000);
            if (add_sec > 0) {
                time->tv_sec += add_sec;
                add_usec -= 1000 * 1000 * add_sec;
            }
            time->tv_usec += add_usec;

            if (time->tv_usec >= 1000 * 1000) {
                time->tv_sec++;
                time->tv_usec -= 1000 * 1000;
            }
        }
    }
}

#endif  /* ! CURRENTIA_TIMEVAL_H_ */
