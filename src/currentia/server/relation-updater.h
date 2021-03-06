// -*- c++ -*-

#ifndef CURRENTIA_RELATION_UPDATER_H_
#define CURRENTIA_RELATION_UPDATER_H_

#include <thread>

#include "currentia/core/relation.h"
#include "currentia/util/print.h"
#include "currentia/core/thread.h"

namespace currentia {
    class RelationUpdater : public thread::Runnable {
        Relation::ptr_t relation_;

        long update_count_;
        useconds_t update_interval_;
        useconds_t update_duration_;

    public:
        RelationUpdater(const Relation::ptr_t& relation,
                        useconds_t update_interval = 1000,
                        useconds_t update_duration = 10):
            relation_(relation),
            update_count_(0),
            update_interval_(update_interval),
            update_duration_(update_duration) {
        }

        ~RelationUpdater() {
#if CURRENTIA_DEBUG
            std::cout << "RelationUpdater::Destructor:: release lock" << std::endl;
#endif
            relation_->unlock();
#if CURRENTIA_DEBUG
            std::cout << "RelationUpdater::Destructor:: release lock done" << std::endl;
#endif
        }

        void set_update_interval(long update_interval) {
            update_interval_ = update_interval;
        }

        double get_update_count() {
            return update_count_;
        }

        void run() {
            while (!stopped()) {
                // randomly select a tuple and update it
                if (update_interval_ > 0)
                    std::this_thread::sleep_for(std::chrono::microseconds(update_interval_));

                relation_->read_write_lock();
                relation_->update();
                std::this_thread::sleep_for(std::chrono::microseconds(update_duration_));
                relation_->unlock();

                update_count_++;
            }

#ifdef CURRENTIA_DEBUG
            std::cout << "RelationUpdater Finished" << std::endl;
#endif
        }
    };
}

#endif  /* ! CURRENTIA_RELATION_UPDATER_H_ */
