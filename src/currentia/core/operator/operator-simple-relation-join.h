// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_SIMPLE_RELATION_JOIN_H_
#define CURRENTIA_OPERATOR_SIMPLE_RELATION_JOIN_H_

#include "currentia/core/tuple.h"
#include "currentia/core/stream.h"
#include "currentia/core/object.h"
#include "currentia/core/window.h"
#include "currentia/core/relation.h"

#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/synopsis.h"

#include "currentia/trait/pointable.h"

namespace currentia {
    // Equi-Join
    class OperatorSimpleRelationJoin: public SingleInputOperator,
                                      public Pointable<OperatorSimpleRelationJoin> {
        Relation::ptr_t relation_;

        std::string join_attribute_name_stream_;
        std::string join_attribute_name_relation_;

        Schema::ptr_t stream_schema_ptr_;
        Schema::ptr_t relation_schema_ptr_;
        Schema::ptr_t joined_schema_ptr_;

        int target_attribute_column_in_stream_;
        int target_attribute_column_in_relation_;

    public:
        typedef Pointable<OperatorSimpleRelationJoin>::ptr_t ptr_t;

        OperatorSimpleRelationJoin(Operator::ptr_t parent_operator_ptr,
                                   const std::string& join_attribute_name_stream,
                                   Relation::ptr_t relation,
                                   const std::string& join_attribute_name_relation):
            SingleInputOperator(parent_operator_ptr),
            // Initialize members
            relation_(relation),
            join_attribute_name_stream_(join_attribute_name_stream),
            join_attribute_name_relation_(join_attribute_name_relation),
            // set schema
            stream_schema_ptr_(parent_operator_ptr_->get_output_schema_ptr()),
            relation_schema_ptr_(relation->get_schema_ptr()),
            joined_schema_ptr_(build_joined_schema_()) {
            set_output_stream(Stream::from_schema(joined_schema_ptr_));
            // get column numbers for fast access
            target_attribute_column_in_stream_ =
                stream_schema_ptr_->get_attribute_index_by_name(join_attribute_name_stream_);
            target_attribute_column_in_relation_ =
                relation_schema_ptr_->get_attribute_index_by_name(join_attribute_name_relation_);
        }

        Tuple::ptr_t next_implementation() {
            while (true) {
                Tuple::ptr_t target_tuple_ptr = parent_operator_ptr_->next();
                if (target_tuple_ptr->is_system_message())
                    return target_tuple_ptr;
                Object target_attribute_value = target_tuple_ptr->get_value_by_index(target_attribute_column_in_stream_);

                Relation::ScopedLock lock = relation_->get_scoped_lock();

                for (std::list<Tuple::ptr_t>::const_iterator relation_iterator = relation_->get_tuple_iterator();
                     relation_iterator != relation_->get_tuple_iterator_end();
                     ++relation_iterator) {
                    Object value_in_relation =
                        (*relation_iterator)->get_value_by_index(target_attribute_column_in_relation_);

                    // check equality
                    bool is_qualified_tuple = target_attribute_value.compare(value_in_relation, Comparator::EQUAL);

                    if (is_qualified_tuple) {
                        // join
                        Tuple::data_t combined_data = target_tuple_ptr->get_concatenated_data(*relation_iterator);
                        return Tuple::create(joined_schema_ptr_, combined_data);
                    }
                }
            }

            // never comes here
            return Tuple::ptr_t();
        }

        void set_current_relation(const Relation::ptr_t& new_relation) {
            relation_ = new_relation;
        }

    private:

        Schema::ptr_t build_joined_schema_() {
            return concat_schemas(stream_schema_ptr_, relation_schema_ptr_);
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_SIMPLE_RELATION_JOIN_H_ */
