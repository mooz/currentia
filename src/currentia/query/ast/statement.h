// -*- c++ -*-

#ifndef CURRENTIA_QUERY_AST_STATEMENT_H_
#define CURRENTIA_QUERY_AST_STATEMENT_H_

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class Statement : private NonCopyable<Statement>,
                      public Pointable<Statement> {
    // public:
    //     virtual ~Statement() = 0;
    };
    // Statement::~Statement() {}
}

#endif  /* ! CURRENTIA_QUERY_AST_STATEMENT_H_ */
