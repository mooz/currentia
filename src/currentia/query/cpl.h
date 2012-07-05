// -*- c++ -*-

#ifndef CURRENTIA_QUERY_CPL_H_
#define CURRENTIA_QUERY_CPL_H_

#include "currentia/core/attribute.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/double-input-operator.h"
#include "currentia/core/operator/operator-abstract-visitor.h"
#include "currentia/core/operator/operator-election.h"
#include "currentia/core/operator/operator-join.h"
#include "currentia/core/operator/operator-mean.h"
#include "currentia/core/operator/operator-projection.h"
#include "currentia/core/operator/operator-selection.h"
#include "currentia/core/operator/operator-simple-relation-join.h"
#include "currentia/core/operator/operator-stream-adapter.h"
#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/pointer.h"
#include "currentia/core/relation.h"
#include "currentia/core/stream.h"
#include "currentia/core/window.h"
#include "currentia/query/cpl-lexer.h"
#include "currentia/trait/pointable.h"

// CPL stands for 'C'urrentia 'P'lan 'L'anguage

namespace currentia {
    class CPLStreamDeclaration;

    struct CPLQueryContainer : public Pointable<CPLQueryContainer> {
        enum ParseState {
            ERROR,
            NEUTRAL
        };

        ParseState state;
        CPLLexer* lexer;

        std::map<std::string, Relation::ptr_t> relations;
        std::map<std::string, Stream::ptr_t> streams;
        std::map<Stream::ptr_t, Operator::ptr_t> root_operators;

        CPLQueryContainer():
            state(NEUTRAL) {
        }

        void define_relation(const std::string& relation_name,
                             const Relation::ptr_t& relation) {
            relations[relation_name] = relation;
        }

        void define_stream(const std::string& stream_name,
                           const Stream::ptr_t& stream,
                           const Operator::ptr_t& root_operator = Operator::ptr_t()) {
            streams[stream_name] = stream;
            root_operators[stream] = root_operator;
        }

        Relation::ptr_t get_relation_by_name(const std::string& relation_name) {
            std::map<std::string, Relation::ptr_t>::const_iterator it = relations.find(relation_name);
            if (it == relations.end())
                throw std::string("Relation ") + relation_name + " is not defined";
            return it->second;
        }

        Stream::ptr_t get_stream_by_name(const std::string& stream_name) {
            std::map<std::string, Stream::ptr_t>::const_iterator it = streams.find(stream_name);
            if (it == streams.end())
                throw std::string("Stream ") + stream_name + " is not defined";
            return it->second;
        }

        Stream::ptr_t get_input_stream_by_name(const std::string& stream_name) {
            Operator::ptr_t op = get_root_operator_by_stream_name(stream_name);
            if (OperatorStreamAdapter* adapter = dynamic_cast<OperatorStreamAdapter*>(op.get())) {
                return adapter->get_input_stream();
            }
            return Stream::ptr_t();
        }

        Operator::ptr_t get_root_operator_for_stream(const Stream::ptr_t& stream) {
            std::map<Stream::ptr_t, Operator::ptr_t>::const_iterator it = root_operators.find(stream);
            if (it == root_operators.end())
                return Operator::ptr_t();
            return it->second;
        }

        Operator::ptr_t get_root_operator_by_stream_name(const std::string& name) {
            return get_root_operator_for_stream(get_stream_by_name(name));
        }
    };

    struct CPLRelationDeclaration {
        std::string relation_name;
        std::list<Attribute*>* attributes_ptr;

        CPLRelationDeclaration(const std::string& relation_name,
                               std::list<Attribute*>* attributes_ptr):
            relation_name(relation_name),
            attributes_ptr(attributes_ptr) {
        }

        Relation::ptr_t get_relation(CPLQueryContainer* query_container) {
            return Relation::ptr_t(new Relation(Schema::from_attribute_pointers(*attributes_ptr)));
        };
        void declare(CPLQueryContainer* query_container) {
            query_container->define_relation(
                relation_name,
                get_relation(query_container)
            );
        }
    };

    struct CPLStreamDeclaration {
        virtual ~CPLStreamDeclaration() = 0;

        virtual std::string get_stream_name() = 0;
        virtual Operator::ptr_t get_operator_tree(CPLQueryContainer* query_container) = 0;
        virtual Stream::ptr_t get_stream(CPLQueryContainer* query_container,
                                         const Operator::ptr_t& operator_tree) = 0;

        void declare(CPLQueryContainer* query_container) {
            Operator::ptr_t operator_tree = get_operator_tree(query_container);

            query_container->define_stream(
                get_stream_name(),
                get_stream(query_container, operator_tree),
                operator_tree
            );
        }
    };
    CPLStreamDeclaration::~CPLStreamDeclaration() {}

    // stream foo(x: int, y: string)
    struct CPLNewStream : public CPLStreamDeclaration {
        std::string stream_name;
        std::list<Attribute*>* attributes_ptr;

        CPLNewStream(const std::string& stream_name,
                     std::list<Attribute*>* attributes_ptr):
            stream_name(stream_name),
            attributes_ptr(attributes_ptr) {
            std::cout << "New Stream " << stream_name << std::endl;
        }

        ~CPLNewStream() {
            // if (attributes_ptr) {
            //     std::list<Attribute*>::const_iterator iter = attributes_ptr->begin();
            //     std::list<Attribute*>::const_iterator iter_end = attributes_ptr->end();
            //     for (; iter != iter_end; ++iter)
            //         delete *iter;
            //     delete attributes_ptr;
            // }
        }

        std::string get_stream_name() {
            return stream_name;
        }

        Operator::ptr_t get_operator_tree(CPLQueryContainer* query_container) {
            Stream::ptr_t stream = Stream::from_schema(Schema::from_attribute_pointers(*attributes_ptr));
            return OperatorStreamAdapter::ptr_t(new OperatorStreamAdapter(stream));
        }

        Stream::ptr_t get_stream(CPLQueryContainer* query_container,
                                 const Operator::ptr_t& operator_tree) {
            return operator_tree->get_output_stream();
        }
    };

    struct CPLField {
        std::string base_name;
        std::string field_name;

        CPLField(const std::string& base_name, const std::string& field_name):
            base_name(base_name),
            field_name(field_name) {
        }
    };

    struct CPLOperationInfo {
        enum Type {
            SELECT,
            PROJECT,
            MEAN,
            SUM,
            ELECT,
            COMBINE
        };

        Type type;
        std::string relation_name;
        Condition::ptr_t condition_ptr;
        std::list<CPLField*>* fields_ptr;
        Window* window_ptr;

        CPLOperationInfo(Type type): type(type) {
        }

        CPLOperationInfo(Type type, Condition* condition_ptr):
            type(type),
            condition_ptr(condition_ptr) {
        }

        CPLOperationInfo(Type type, std::list<CPLField*>* fields_ptr, Window* window_ptr = NULL):
            type(type),
            fields_ptr(fields_ptr),
            window_ptr(window_ptr) {
        }

        CPLOperationInfo(Type type, const std::string& relation_name, Condition* condition_ptr):
            type(type),
            relation_name(relation_name),
            condition_ptr(condition_ptr) {
        }

        ~CPLOperationInfo() __attribute__((optimize(0))) {
            // if (window_ptr)
            //     delete window_ptr;

            // if (fields_ptr) {
            //     std::list<CPLField*>::const_iterator iter = fields_ptr->begin();
            //     std::list<CPLField*>::const_iterator iter_end = fields_ptr->end();
            //     for (; iter != iter_end; ++iter)
            //         delete *iter;
            //     delete fields_ptr;
            // }
        }

        Operator::ptr_t to_operator(const Operator::ptr_t& parent_operator,
                                    CPLQueryContainer* query_container) {
            Operator* op = NULL;

            switch (type) {
            case SELECT:
                op = new OperatorSelection(parent_operator, condition_ptr);
                break;
            case PROJECT: {
                std::list<std::string> attribute_names;
                std::list<CPLField*>::const_iterator iter = fields_ptr->begin();
                std::list<CPLField*>::const_iterator iter_end = fields_ptr->end();
                for (; iter != iter_end; ++iter)
                    attribute_names.push_back((*iter)->field_name);
                op = new OperatorProjection(parent_operator, attribute_names);
                break;
            }
            case MEAN:
                op = new OperatorMean(parent_operator, *window_ptr, fields_ptr->front()->field_name);
                break;
            case SUM:
                throw "SUM is not supported for now";
                break;
            case ELECT:
                op = new OperatorElection(parent_operator, *window_ptr);
                break;
            case COMBINE:
                op = new OperatorSimpleRelationJoin(
                    parent_operator,
                    query_container->get_relation_by_name(relation_name),
                    condition_ptr
                );
                break;
            default:
                break;
            }

            return Operator::ptr_t(op);
        }
    };

    // stream foo from bar { ... }
    struct CPLDerivedStream : public CPLStreamDeclaration {
        std::string stream_name;
        std::list<CPLOperationInfo*>* operations_ptr;

        CPLDerivedStream() {
            operations_ptr = NULL;
        }

        ~CPLDerivedStream() {
            // if (operations_ptr) {
            //     std::list<CPLOperationInfo*>::const_iterator iter = operations_ptr->begin();
            //     std::list<CPLOperationInfo*>::const_iterator iter_end = operations_ptr->end();
            //     for (; iter != iter_end; ++iter)
            //         delete *iter;
            //     delete operations_ptr;
            // }
        }

        std::string get_stream_name() {
            return stream_name;
        }

        bool is_anonymous_stream() {
            return stream_name == "";
        }

        Operator::ptr_t get_operator_tree(CPLQueryContainer* query_container) {
            Operator::ptr_t current_root = get_source_operator(query_container);

            if (operations_ptr) {
                std::list<CPLOperationInfo*>::const_iterator iter = operations_ptr->begin();
                std::list<CPLOperationInfo*>::const_iterator iter_end = operations_ptr->end();
                for (; iter != iter_end; ++iter) {
                    current_root = (*iter)->to_operator(current_root, query_container);
                }
            }

            return current_root;
        }

        Stream::ptr_t get_stream(CPLQueryContainer* query_container,
                                 const Operator::ptr_t& operator_tree) {
            return operator_tree->get_output_stream();
        }

        virtual Operator::ptr_t get_source_operator(CPLQueryContainer* query_container) = 0;
    };

    struct CPLJoinedStream : public CPLDerivedStream {
        Operator::ptr_t* left_operator_ptr_;
        Operator::ptr_t* right_operator_ptr_;
        Condition::ptr_t condition_ptr;

        Window window1;
        Window window2;

        CPLJoinedStream(Operator::ptr_t* left_operator_ptr,
                        Window window1,
                        Operator::ptr_t* right_operator_ptr,
                        Window window2,
                        Condition* condition_ptr):
            CPLDerivedStream(),
            left_operator_ptr_(left_operator_ptr),
            right_operator_ptr_(right_operator_ptr),
            condition_ptr(condition_ptr) {
        }

        ~CPLJoinedStream() {
            // delete left_operator_ptr_;
            // delete right_operator_ptr_;
        }

        Operator::ptr_t get_source_operator(CPLQueryContainer* query_container) {
            return OperatorJoin::ptr_t(
                new OperatorJoin(
                    Operator::ptr_t(*left_operator_ptr_),
                    window1,
                    Operator::ptr_t(*right_operator_ptr_),
                    window2,
                    condition_ptr
                )
            );
        }
    };

    struct CPLSingleStream : public CPLDerivedStream {
        Operator::ptr_t* root_operator_;

        CPLSingleStream(Operator::ptr_t* root_operator):
            CPLDerivedStream(),
            root_operator_(root_operator) {
        }

        ~CPLSingleStream() {
            // delete root_operator_;
        }

        Operator::ptr_t get_source_operator(CPLQueryContainer* query_container) {
            return Operator::ptr_t(*root_operator_);
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_CPL_H_ */
