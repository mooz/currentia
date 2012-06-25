// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_VISITOR_SERIALIZER_H_
#define CURRENTIA_OPERATOR_VISITOR_SERIALIZER_H_

#include <iostream>

#include "currentia/core/operator/double-input-operator.h"
#include "currentia/core/operator/operator-abstract-visitor.h"
#include "currentia/core/operator/operator-stream-adapter.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/pointer.h"

namespace currentia {
    // Serialize a operator tree by sorting topological sort algorithm
    class OperatorVisitorSerializer : public OperatorAbstractVisitor,
                                      public Pointable<OperatorVisitorSerializer> {
        std::list<Operator*> operators_;

    public:
        typedef Pointable<OperatorVisitorSerializer>::ptr_t ptr_t;

        OperatorVisitorSerializer() {
        }

        void visit(SingleInputOperator& op) {
            std::clog << "Visit Single Operator!" << std::endl;
            operators_.push_back(&op);
            dispatch_(op.get_parent_operator());
        }

        void visit(DoubleInputOperator& op) {
            std::clog << "Visit Binary Operator!" << std::endl;
            operators_.push_back(&op);
            dispatch_(op.get_parent_left_operator());
            dispatch_(op.get_parent_right_operator());
        }

        void visit(OperatorStreamAdapter& op) {
            std::clog << "Visit Stream Adapter!" << std::endl;
            operators_.push_back(&op);
        }

    private:
        void dispatch_(const Operator::ptr_t& op) {
            if (std::dynamic_pointer_cast<SingleInputOperator>(op)) {
                std::dynamic_pointer_cast<SingleInputOperator>(op)->accept(this);
            } else if (std::dynamic_pointer_cast<DoubleInputOperator>(op)) {
                std::dynamic_pointer_cast<DoubleInputOperator>(op)->accept(this);
            } else if (std::dynamic_pointer_cast<OperatorStreamAdapter>(op)) {
                std::dynamic_pointer_cast<OperatorStreamAdapter>(op)->accept(this);
            }

            throw "Unhandled operator";
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_VISITOR_SERIALIZER_H_ */
