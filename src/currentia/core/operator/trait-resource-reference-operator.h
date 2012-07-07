// -*- c++ -*-

#ifndef CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_
#define CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_

#include "currentia/core/operator/operator.h"
#include "currentia/core/relation.h"

namespace currentia {
    class TraitResourceReferenceOperator {
        std::vector<Relation::ptr_t> resource_list_;

    protected:
        TraitResourceReferenceOperator(const std::vector<Relation::ptr_t>& resource_list):
            resource_list_(resource_list),
            first_time_in_txn_(true) {
        }

        virtual ~TraitResourceReferenceOperator() = 0;

    public:
        std::vector<Relation::ptr_t>::const_iterator resource_list_begin() {
            return resource_list_.begin();
        }

        std::vector<Relation::ptr_t>::const_iterator resource_list_end() {
            return resource_list_.end();
        }

        // Locks

        void get_recursive_locks() {
            auto iter = resource_list_begin();
            auto iter_end = resource_list_end();
            for (; iter != iter_end; ++iter) {
                (*iter)->read_write_lock();
            }
        }

        void release_recursive_locks() {
            auto iter = resource_list_begin();
            auto iter_end = resource_list_end();
            for (; iter != iter_end; ++iter) {
                (*iter)->unlock();
            }
        }

        // Operation / Txn

        bool first_time_in_txn_;
        void reference_operation_begin(Operator::CCMode cc_mode) {
            if (cc_mode != Operator::PESSIMISTIC_2PL || first_time_in_txn_)
                get_recursive_locks();
            first_time_in_txn_ = false;
        }

        void reference_operation_end(Operator::CCMode cc_mode) {
            if (cc_mode != Operator::PESSIMISTIC_2PL)
                release_recursive_locks();
        }

        // void transaction_begin() {
        //     first_time_in_txn_ = false;
        // }

        void transaction_end(Operator::CCMode cc_mode) {
            if (cc_mode == Operator::PESSIMISTIC_2PL) {
                release_recursive_locks();
            }
            first_time_in_txn_ = false;
        }
    };

    TraitResourceReferenceOperator::~TraitResourceReferenceOperator() {}
}

#endif  /* ! CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_ */
