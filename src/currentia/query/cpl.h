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
    struct CPLQueryContainer : public Pointable<CPLQueryContainer> {
        enum ParseState {
            ERROR,
            NEUTRAL
        };

        ParseState state;
        CPLLexer* lexer;

        std::map<std::string, Relation::ptr_t> relations;
        std::map<std::string, Stream::ptr_t> streams;

        CPLQueryContainer():
            state(NEUTRAL) {
        }

        void define_relation(const std::string& relation_name,
                             const Relation::ptr_t& relation) {
            relations[relation_name] = relation;
        }

        void define_stream(const std::string& stream_name,
                           const Stream::ptr_t& stream) {
            streams[stream_name] = stream;
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
        virtual Stream::ptr_t get_stream(CPLQueryContainer* query_container) = 0;
        void declare(CPLQueryContainer* query_container) {
            query_container->define_stream(
                get_stream_name(),
                get_stream(query_container)
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
            if (attributes_ptr) {
                std::list<Attribute*>::const_iterator iter = attributes_ptr->begin();
                std::list<Attribute*>::const_iterator iter_end = attributes_ptr->end();
                for (; iter != iter_end; ++iter)
                    delete *iter;
                delete attributes_ptr;
            }
        }

        std::string get_stream_name() {
            return stream_name;
        }

        Stream::ptr_t get_stream(CPLQueryContainer* query_container) {
            return Stream::from_schema(Schema::from_attribute_pointers(*attributes_ptr));
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

        ~CPLOperationInfo() {
            if (window_ptr)
                delete window_ptr;

            if (fields_ptr) {
                std::list<CPLField*>::const_iterator iter = fields_ptr->begin();
                std::list<CPLField*>::const_iterator iter_end = fields_ptr->end();
                for (; iter != iter_end; ++iter)
                    delete *iter;
                delete fields_ptr;
            }
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
                op = new OperatorElection(parent_operator, window_ptr->width);
                break;
            case COMBINE:
                op = new OperatorSimpleRelationJoin(parent_operator,
                                                    query_container->get_relation_by_name(relation_name),
                                                    condition_ptr);
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
            if (operations_ptr) {
                std::list<CPLOperationInfo*>::const_iterator iter = operations_ptr->begin();
                std::list<CPLOperationInfo*>::const_iterator iter_end = operations_ptr->end();
                for (; iter != iter_end; ++iter)
                    delete *iter;
                delete operations_ptr;
            }
        }

        std::string get_stream_name() {
            return stream_name;
        }

        Stream::ptr_t get_stream(CPLQueryContainer* query_container) {
            Operator::ptr_t current_root = get_source_operator(query_container);

            if (operations_ptr) {
                std::list<CPLOperationInfo*>::const_iterator iter = operations_ptr->begin();
                std::list<CPLOperationInfo*>::const_iterator iter_end = operations_ptr->end();
                for (; iter != iter_end; ++iter) {
                    current_root = (*iter)->to_operator(current_root, query_container);
                }
            }

            std::cout << stream_name << ": " << current_root->toString() << std::endl;

            return current_root->get_output_stream();
        }

        virtual Operator::ptr_t get_source_operator(CPLQueryContainer* query_container) = 0;
    };

    struct CPLJoinedStream : public CPLDerivedStream {
        std::string ancestor_stream_name1;
        std::string ancestor_stream_name2;
        ConditionAttributeComparator::ptr_t condition;

        Window window1;
        Window window2;

        CPLJoinedStream(const std::string& left_name,
                        Window window1,
                        const std::string& right_name,
                        Window window2,
                        ConditionAttributeComparator* condition):
            CPLDerivedStream(),
            ancestor_stream_name1(left_name),
            ancestor_stream_name2(right_name),
            condition(condition) {
        }

        ~CPLJoinedStream() {
        }

        Operator::ptr_t get_source_operator(CPLQueryContainer* query_container) {
            Stream::ptr_t ancestor_stream1 =
                query_container->get_stream_by_name(ancestor_stream_name1);
            Stream::ptr_t ancestor_stream2 =
                query_container->get_stream_by_name(ancestor_stream_name2);

            return OperatorJoin::ptr_t(
                new OperatorJoin(
                    OperatorStreamAdapter::ptr_t(new OperatorStreamAdapter(ancestor_stream1)),
                    window1,
                    OperatorStreamAdapter::ptr_t(new OperatorStreamAdapter(ancestor_stream2)),
                    window2,
                    condition
                )
            );
        }
    };

    struct CPLSingleStream : public CPLDerivedStream {
        std::string ancestor_stream_name;

        CPLSingleStream(const std::string& ancestor_stream_name):
            CPLDerivedStream(),
            ancestor_stream_name(ancestor_stream_name) {
        }

        Operator::ptr_t get_source_operator(CPLQueryContainer* query_container) {
            Stream::ptr_t ancestor_stream =
                query_container->get_stream_by_name(ancestor_stream_name);
            return OperatorStreamAdapter::ptr_t(
                new OperatorStreamAdapter(ancestor_stream)
            );
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_CPL_H_ */
