// -*- c++ -*-

#ifndef CURRENTIA_TIME_H_
#define CURRENTIA_TIME_H_

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>

#include "currentia/util/log.h"

namespace currentia {
    namespace time {
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

        double usec_to_sec(useconds_t usec) {
            return static_cast<double>(usec) * 0.001 * 0.001;
        }

        double get_current_time_in_seconds() {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return static_cast<double>(tv.tv_sec) + usec_to_sec(tv.tv_usec);
        }

#define TIME_IT(ELAPSED_TIME_VARIABLE)                                  \
        double ELAPSED_TIME_VARIABLE = 0;                               \
        if (time::AutoTimer autotimer = time::AutoTimer(ELAPSED_TIME_VARIABLE))

        class AutoTimer {
            double begin_time_;
            double& elapsed_time_var_ref_;

        public:
            AutoTimer(double& elapsed_time_var_ref):
                elapsed_time_var_ref_(elapsed_time_var_ref) {
                begin_time_ = get_current_time_in_seconds();
            }

            ~AutoTimer() {
                elapsed_time_var_ref_ = get_current_time_in_seconds() - begin_time_;
            }

            operator bool() const {
                return true;
            }
        };
    }
}

#endif  /* ! CURRENTIA_TIME_H_ */
