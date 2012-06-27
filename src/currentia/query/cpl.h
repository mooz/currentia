// -*- c++ -*-

#ifndef CURRENTIA_CPL_H_
#define CURRENTIA_CPL_H_

#include "currentia/core/attribute.h"
#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/double-input-operator.h"
#include "currentia/core/operator/operator-abstract-visitor.h"
#include "currentia/core/operator/operator-stream-adapter.h"
#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/pointer.h"
#include "currentia/core/relation.h"
#include "currentia/core/stream.h"
#include "currentia/core/window.h"

// CPL stands for 'C'urrentia 'P'lan 'L'anguage

namespace currentia {
    struct CPLContainer {
        std::map<std::string, Relation::ptr_t> relations;
        std::map<std::string, Stream::ptr_t> streams;
        int current_token;
        std::string* current_token_string;
    };

    struct CPLStreamDeclaration {
        // std::string stream_name;
        // CPLStreamDeclaration(const std::string& stream_name):
        //     stream_name(stream_name) {
        // }

        virtual ~CPLStreamDeclaration() = 0;
    };
    CPLStreamDeclaration::~CPLStreamDeclaration() {}

    struct CPLNewStream : public CPLStreamDeclaration {
        std::string stream_name;
        std::list<Attribute*>* attributes_ptr;

        // CPLNewStream(const std::string& stream_name):
        //     stream_name(stream_name) {
        // }
    };

    // derived_from
    struct CPLDerivedStream : public CPLStreamDeclaration {
        enum Type {
            JOINED_STREAM,
            SINGLE_STREAM
        };

        std::string stream_name;

        std::string name1;
        std::string name2;
        Condition* condition;
        Type type;

        CPLDerivedStream(const std::string& left_name, const std::string& right_name, Condition* condition):
            name1(left_name),
            name2(right_name),
            condition(condition) {
            type = JOINED_STREAM;
        }

        CPLDerivedStream(const std::string& derived_name):
            name1(derived_name),
            name2("") {
            type = SINGLE_STREAM;
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
            ELECT
        };

        Type type;
        Condition* condition_ptr;
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
    };
}

#endif  /* ! CURRENTIA_CPL_H_ */
