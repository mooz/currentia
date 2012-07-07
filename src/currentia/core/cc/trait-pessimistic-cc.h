// -*- c++ -*-

#ifndef CURRENTIA_TRAIT_PESSIMISTIC_CC_H_
#define CURRENTIA_TRAIT_PESSIMISTIC_CC_H_

namespace currentia {
    class TraitPessimisticCC {
    protected:
        int txn_joint_count_;
        int commit_count_;

    public:
        TraitPessimisticCC(int txn_joint_count):
            txn_joint_count_(txn_joint_count),
            commit_count_(0)
            {
        }
        virtual ~TraitPessimisticCC() = 0;

    protected:
        void reset_commit_count_() {
            commit_count_ = 0;
        }

        bool has_transaction_end_() {
            return commit_count_ >= txn_joint_count_;
        }
    };

    TraitPessimisticCC::~TraitPessimisticCC() {}
}

#endif  /* ! CURRENTIA_TRAIT_PESSIMISTIC_CC_H_ */
