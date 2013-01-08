// -*- c++ -*-

#ifndef CURRENTIA_RELATION_UPDATER_H_
#define CURRENTIA_RELATION_UPDATER_H_

#include "currentia/core/relation.h"
#include "currentia/util/print.h"
#include "currentia/core/thread.h"

namespace currentia {
    class RelationUpdater : public thread::Runnable {
        Relation::ptr_t relation_;

        long update_count_;
        useconds_t update_interval_;
        useconds_t update_duration_;
        bool is_runnable_;

    public:
        RelationUpdater(const Relation::ptr_t& relation,
                        useconds_t update_interval = 1000,
                        useconds_t update_duration = 1000):
            relation_(relation),
            update_count_(0),
            update_interval_(update_interval),
            is_runnable_(true) {
        }

        void set_update_interval(long update_interval) {
            update_interval_ = update_interval;
        }

        double get_update_count() {
            return update_interval_;
        }

        void run() {
            while (is_runnable_) {
                // randomly select a tuple and update it
                if (update_interval_ > 0)
                    usleep(update_interval_);

                relation_->read_write_lock();
                relation_->update();
                usleep(update_duration_);
                relation_->unlock();

                update_count_++;
            }
        }

        void stop() {
            is_runnable_ = false;
        }
    };
}

#endif  /* ! CURRENTIA_RELATION_UPDATER_H_ */
