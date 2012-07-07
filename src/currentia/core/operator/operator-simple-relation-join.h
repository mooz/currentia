// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_SIMPLE_RELATION_JOIN_H_
#define CURRENTIA_OPERATOR_SIMPLE_RELATION_JOIN_H_

#include "currentia/core/object.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/trait-resource-reference-operator.h"
#include "currentia/core/operator/synopsis.h"
#include "currentia/core/relation.h"
#include "currentia/core/stream.h"
#include "currentia/core/thread.h"
#include "currentia/core/tuple.h"
#include "currentia/core/window.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    // Equi-Join
    class OperatorSimpleRelationJoin: public SingleInputOperator,
                                      public TraitResourceReferenceOperator,
                                      public Pointable<OperatorSimpleRelationJoin> {
        Relation::ptr_t relation_;
        Condition::ptr_t join_condition_;

        std::string join_attribute_name_stream_;
        std::string join_attribute_name_relation_;

        Schema::ptr_t stream_schema_ptr_;
        Schema::ptr_t relation_schema_ptr_;
        Schema::ptr_t joined_schema_ptr_;

    public:
        typedef Pointable<OperatorSimpleRelationJoin>::ptr_t ptr_t;

        OperatorSimpleRelationJoin(Operator::ptr_t parent_operator_ptr,
                                   Relation::ptr_t relation,
                                   const Condition::ptr_t& join_condition):
            SingleInputOperator(parent_operator_ptr),
            TraitResourceReferenceOperator({ relation }),
            // Initialize members
            relation_(relation),
            join_condition_(join_condition),
            // set schema
            stream_schema_ptr_(parent_operator_ptr_->get_output_schema_ptr()),
            relation_schema_ptr_(relation->get_schema()),
            joined_schema_ptr_(build_joined_schema_()) {
            set_output_stream(Stream::from_schema(joined_schema_ptr_));
            // obey schema
            join_condition_->obey_schema(
                parent_operator_ptr->get_output_stream()->get_schema(),
                relation->get_schema()
            );
            // snapshot pointers
            set_reference_to_snapshots({ &relation_ });
        }

        void process_single_input(Tuple::ptr_t input_tuple) {
            reference_operation_begin(cc_mode_);

            // TODO: Use index
            auto relation_iter = relation_->get_tuple_iterator();
            auto relation_iter_end = relation_->get_tuple_iterator_end();
            for (; relation_iter != relation_iter_end; ++relation_iter) {
                if (join_condition_->check(input_tuple, *relation_iter)) {
                    Tuple::data_t combined_data = input_tuple->get_concatenated_data(*relation_iter);
                    Tuple::ptr_t combined_tuple = Tuple::create(joined_schema_ptr_,
                                                                combined_data,
                                                                input_tuple->get_arrived_time());
#ifdef CURRENTIA_ENABLE_TRANSACTION
                    combined_tuple->set_hwm(input_tuple->get_hwm());
                    combined_tuple->set_referenced_version_number(relation_, relation_->get_version_number());
#endif

                    output_tuple(combined_tuple);
                }
            }

            reference_operation_end(cc_mode_);
        }

        void set_current_relation(const Relation::ptr_t& new_relation) {
            relation_ = new_relation;
        }

    private:

        Schema::ptr_t build_joined_schema_() {
            return concat_schemas(stream_schema_ptr_, relation_schema_ptr_);
        }

    public:
        std::string toString() const {
            std::stringstream ss;
            ss << parent_operator_ptr_->toString() << "\n -> " << get_name()
               << "(" << join_condition_->toString() << ")";
            return ss.str();
        }

        std::string get_name() const {
            return std::string("Relation-Join");
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_SIMPLE_RELATION_JOIN_H_ */
