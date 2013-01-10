#ifndef CURRENTIA_OPERATOR_FORWARD_DECLARATION_H_
#define CURRENTIA_OPERATOR_FORWARD_DECLARATION_H_

#define CURRENTIA_OPERATOR_LIST(OPERATION)                              \
    OPERATION(SingleInputOperator)                                      \
    OPERATION(DoubleInputOperator)                                      \
    OPERATION(OperatorStreamAdapter)                                    \

#define CURRENTIA_OPERATOR_DECLARE(OPERATOR)    \
    class OPERATOR;

#define CURRENTIA_OPERATOR_DEFINE_VISIT(OPERATOR)       \
    virtual void visit(OPERATOR* op) = 0;

#define CURRENTIA_DEFINE_OPERATOR_DISPATCHER() \
    void dispatch(Operator* op) {                                       \
        if (dynamic_cast<SingleInputOperator*>(op)) {                   \
            dynamic_cast<SingleInputOperator*>(op)->accept(this);       \
        } else if (dynamic_cast<DoubleInputOperator*>(op)) {            \
            dynamic_cast<DoubleInputOperator*>(op)->accept(this);       \
        } else if (dynamic_cast<OperatorStreamAdapter*>(op)) {          \
            dynamic_cast<OperatorStreamAdapter*>(op)->accept(this);     \
        } else {                                                        \
            throw (                                                     \
                std::string("Unhandled operator: ")                     \
                + op->toString()                                        \
            ).c_str();                                                  \
        }                                                               \
    }                                                                   \

#endif  /* ! CURRENTIA_OPERATOR_FORWARD_DECLARATION_H_ */
