// -*- c++ -*-

#ifndef CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_
#define CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_

#include "currentia/core/relation.h"

namespace currentia {
    class TraitResourceReferenceOperator {
        std::vector<Relation::ptr_t> resource_list_;

    protected:
        TraitResourceReferenceOperator(const std::vector<Relation::ptr_t>& resource_list):
            resource_list_(resource_list) {
        }

        virtual ~TraitResourceReferenceOperator() = 0;

    public:
        std::vector<Relation::ptr_t>::const_iterator resource_list_begin() {
            return resource_list_.begin();
        }

        std::vector<Relation::ptr_t>::const_iterator resource_list_end() {
            return resource_list_.end();
        }
    };

    TraitResourceReferenceOperator::~TraitResourceReferenceOperator() {}
}

#endif  /* ! CURRENTIA_TRAIT_RESOURCE_REFERENCE_OPERATOR_H_ */
