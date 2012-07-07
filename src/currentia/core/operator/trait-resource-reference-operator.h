// -*- c++ -*-

#ifndef CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_
#define CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_

#include "currentia/core/operator/operator.h"
#include "currentia/core/relation.h"

namespace currentia {
    class TraitResourceReferenceOperator {
        std::vector<Relation::ptr_t*> snapshot_ptr_list_;
        std::vector<Relation::ptr_t> original_resource_list_;

    protected:
        TraitResourceReferenceOperator(const std::vector<Relation::ptr_t>& original_resource_list):
            original_resource_list_(original_resource_list),
            first_time_in_txn_(true) {
        }

        virtual ~TraitResourceReferenceOperator() = 0;

    public:
        void set_reference_to_snapshots(const std::vector<Relation::ptr_t*>& snapshot_ptr_list) {
            if (original_resource_list_.size() != snapshot_ptr_list.size())
                throw "set_reference_to_snapshots: number of snapshot doesn't equal to the number of actual resources";
            snapshot_ptr_list_ = snapshot_ptr_list;
        }

        // Locks

        void get_recursive_locks() {
            auto iter = original_resource_list_.begin();
            auto iter_end = original_resource_list_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->read_write_lock();
            }
        }

        void release_recursive_locks() {
            auto iter = original_resource_list_.begin();
            auto iter_end = original_resource_list_.end();
            for (; iter != iter_end; ++iter) {
                (*iter)->unlock();
            }
        }

        // Operation / Txn

        bool first_time_in_txn_;
        void reference_operation_begin(Operator::CCMode cc_mode) {
            switch (cc_mode) {
            case Operator::PESSIMISTIC_2PL:
                if (first_time_in_txn_)
                    get_recursive_locks();
                break;
            case Operator::PESSIMISTIC_SNAPSHOT:
                get_recursive_locks();
                if (first_time_in_txn_)
                    refresh_snapshots();
                break;
            default:
                get_recursive_locks();
            }
            first_time_in_txn_ = false;
        }

        void reference_operation_end(Operator::CCMode cc_mode) {
            switch (cc_mode) {
            case Operator::PESSIMISTIC_2PL:
                break;
            case Operator::PESSIMISTIC_SNAPSHOT:
            default:
                release_recursive_locks();
                break;
            }
        }

        void refresh_snapshots() {
            auto snapshot_ptr_iter = snapshot_ptr_list_.begin();
            auto original_resource_iter = original_resource_list_.begin();
            auto original_resource_iter_end = original_resource_list_.end();

            while (original_resource_iter != original_resource_iter_end) {
                **snapshot_ptr_iter = (*original_resource_iter)->copy();
                ++snapshot_ptr_iter;
                ++original_resource_iter;
            }
        }

        // void transaction_begin() {
        //     first_time_in_txn_ = false;
        // }

        void transaction_end(Operator::CCMode cc_mode) {
            switch (cc_mode) {
            case Operator::PESSIMISTIC_2PL:
                release_recursive_locks();
                break;
            case Operator::PESSIMISTIC_SNAPSHOT:
                std::clog << "}" << std::endl;
                break;
            default:
                break;
            }
            first_time_in_txn_ = true;
        }
    };

    TraitResourceReferenceOperator::~TraitResourceReferenceOperator() {}
}

#endif  /* ! CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_ */
