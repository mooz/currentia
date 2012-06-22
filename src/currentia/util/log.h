// -*- c++ -*-

#ifndef CURRENTIA_LOG_H_
#define CURRENTIA_LOG_H_

#include <cstdio>
#include <cstring>
#include <ostream>
#include <streambuf>
#include <string>
#include <syslog.h>

#include "currentia/trait/non-copyable.h"

namespace currentia {
    namespace core {
        // http://stackoverflow.com/questions/2638654/redirect-c-stdclog-to-syslog-on-unix

        enum LogPriority {
            kLogEmerg   = LOG_EMERG,   // system is unusable
            kLogAlert   = LOG_ALERT,   // action must be taken immediately
            kLogCrit    = LOG_CRIT,    // critical conditions
            kLogErr     = LOG_ERR,     // error conditions
            kLogWarning = LOG_WARNING, // warning conditions
            kLogNotice  = LOG_NOTICE,  // normal, but significant, condition
            kLogInfo    = LOG_INFO,    // informational message
            kLogDebug   = LOG_DEBUG    // debug-level message
        };

        class Log: public std::basic_streambuf<char, std::char_traits<char> > {
        public:
            explicit Log(std::string ident, int facility = LOG_USER) {
                facility_ = facility;
                priority_ = LOG_DEBUG;
                openlog(ident_.c_str(), LOG_PID, facility_);
            }

        protected:
            int sync() {
                if (buffer_.length()) {
                    syslog(priority_, "%s", buffer_.c_str());
                    buffer_.erase();
                    priority_ = LOG_DEBUG; // default to debug for each message
                }
                return 0;
            }

            int overflow(int c) {
                if (c == EOF)
                    sync();
                else
                    buffer_ += static_cast<char>(c);
                return c;
            }

        private:
            friend std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);
            std::string buffer_;
            int facility_;
            int priority_;
            std::string ident_;
        };

        std::ostream& operator<<(std::ostream& os, const LogPriority& log_priority) {
            reinterpret_cast<Log*>(os.rdbuf())->priority_ = static_cast<int>(log_priority);
            return os;
        }
    }
}

#endif  /* ! CURRENTIA_LOG_H_ */
