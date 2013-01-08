// -*- c++ -*-

#ifndef CURRENTIA_RELATION_UPDATER_H_
#define CURRENTIA_RELATION_UPDATER_H_

#include "currentia/core/relation.h"
#include "currentia/util/print.h"

namespace currentia {
    class RelationUpdater {
        Relation::ptr_t relation_;

        long update_count_;
        long update_interval_;

    public:
        RelationUpdater(const Relation::ptr_t& relation):
            relation_(relation),
            update_count_(0),
            update_interval_(0) {
        }

        void run() {
            while (true) {
                // randomly select a tuple and update it
                if (update_interval_ > 0)
                    usleep(update_interval_);

                relation_->read_write_lock();
                relation_->update();
                usleep(update_interval_);
                relation_->unlock();

                update_count_++;
            }
        }
    };
}

#endif  /* ! CURRENTIA_RELATION_UPDATER_H_ */
