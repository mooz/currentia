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
        std::vector<Operator*> operators_;

    public:
        typedef Pointable<OperatorVisitorSerializer>::ptr_t ptr_t;

        OperatorVisitorSerializer() {
        }

        const std::vector<Operator*>& get_sorted_operators() {
            return operators_;
        }

        void visit(SingleInputOperator& op) {
            std::clog << "Visit Single Operator: " << op.toString() << std::endl;
            operators_.push_back(&op);
            dispatch(op.get_parent_operator().get());
        }

        void visit(DoubleInputOperator& op) {
            std::clog << "Visit Binary Operator: " << op.toString() << std::endl;
            operators_.push_back(&op);
            dispatch(op.get_parent_left_operator().get());
            dispatch(op.get_parent_right_operator().get());
        }

        void visit(OperatorStreamAdapter& op) {
            std::clog << "Visit Stream Adapter: " << op.toString() << std::endl;
            operators_.push_back(&op);
        }

        void dispatch(Operator* op) {
            if (dynamic_cast<SingleInputOperator*>(op)) {
                dynamic_cast<SingleInputOperator*>(op)->accept(this);
            } else if (dynamic_cast<DoubleInputOperator*>(op)) {
                dynamic_cast<DoubleInputOperator*>(op)->accept(this);
            } else if (dynamic_cast<OperatorStreamAdapter*>(op)) {
                dynamic_cast<OperatorStreamAdapter*>(op)->accept(this);
            } else {
                throw (std::string("Unhandled operator: ") + op->toString()).c_str();
            }
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_VISITOR_SERIALIZER_H_ */
