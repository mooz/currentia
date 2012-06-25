#ifndef CURRENTIA_OPERATOR_FORWARD_DECLARATION_H_
#define CURRENTIA_OPERATOR_FORWARD_DECLARATION_H_

#define CURRENTIA_OPERATOR_LIST(OPERATION)                              \
    OPERATION(SingleInputOperator)                                      \
    OPERATION(DoubleInputOperator)                                      \
    OPERATION(OperatorStreamAdapter)                                    \

#define CURRENTIA_OPERATOR_DECLARE(OPERATOR)    \
    class OPERATOR;

#define CURRENTIA_OPERATOR_DEFINE_VISIT(OPERATOR)       \
    virtual void visit(OPERATOR& op) = 0;

#endif  /* ! CURRENTIA_OPERATOR_FORWARD_DECLARATION_H_ */
