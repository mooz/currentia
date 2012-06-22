#ifndef CURRENTIA_QUERY_AST_FORWARD_DECLARATION_H_
#define CURRENTIA_QUERY_AST_FORWARD_DECLARATION_H_

#define CURRENTIA_QUERY_AST_NODE_LIST(OPERATION)        \
    OPERATION(Create)                                   \
    /* OPERATION(SelectionNode)                            \ */
    /* OPERATION(FromSelectionNode)                        \ */
    /* OPERATION(FromStreamNode) */

#define CURRENTIA_QUERY_AST_OPERATION_DECLARE(NODE)     \
    class NODE;

#define CURRENTIA_QUERY_AST_OPERATION_DEFINE_VISIT(NODE)     \
    virtual void visit(NODE& node) = 0;

#endif  /* ! CURRENTIA_QUERY_AST_FORWARD_DECLARATION_H_ */
