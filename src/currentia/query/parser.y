%token_type { std::string* }

%extra_argument { CPLContainer* cpl_container }

%name CPLParse
%token_prefix TOKEN_

%include {
#include <iostream>
#include <string>
#include <assert.h>

#include "currentia/core/operator/comparator.h"
#include "currentia/core/operator/condition.h"
#include "currentia/query/cpl.h"
#include "currentia/query/parser.h"

#define LOG(str) std::cerr << str << std::endl
}

%syntax_error {
    std::cerr << "Syntax error near: " << *(cpl_container->current_token_string) << std::endl;
}

%parse_failure {
    std::cerr << "Failed to parse input" << std::endl;
}

%parse_accept {
    std::cerr << "Parse accept!!" << std::endl;
}

%token_destructor {
    delete $$;
}

plan ::= statements. {
    std::cerr << "OK, parsed statements" << std::endl;
}
plan ::= UNKNOWN. {
    std::cerr << "Unknown token" << std::endl;
}

statements ::= statements statement.
statements ::= statement.

statement ::= define_relation.
statement ::= define_stream.

// ------------------------------------------------------------
// Relation definition
// ------------------------------------------------------------

// %type define_relation { Operator* }
define_relation ::= RELATION NAME(RelationName) LPAREN attributes(Attributes) RPAREN. {
    std::cout << "Relation " << *RelationName << " is defined." << std::endl;
}

// ------------------------------------------------------------
// Stream definition
// ------------------------------------------------------------

define_stream ::= new_stream.
define_stream ::= derived_stream.

// ------------------------------------------------------------
// Stream definition / New Stream
// ------------------------------------------------------------

new_stream ::= STREAM NAME(StreamName) LPAREN attributes(Attributes) RPAREN. {
    std::cout << "New Stream " << *StreamName << " is defined." << std::endl;
}

// ------------------------------------------------------------
// Stream definition / Derived Stream
// ------------------------------------------------------------

// Create a stream from a sequence of operation
derived_stream ::= STREAM NAME(StreamName) FROM derived_from LBRACE operations RBRACE. {
    LOG("Derived Stream");
    // std::cout << "Derived Stream " << *StreamName << " is defined." << std::endl;
}

derived_from ::= NAME(A) COMMA NAME(B) LBRACKET condition RBRACKET. {
    std::cout << "Join " << *A << " with " << *B << std::endl;
}
derived_from ::= NAME.

// ------------------------------------------------------------
// Attributes
// ------------------------------------------------------------

%type attributes { std::list<Attribute*>* }
%destructor attributes { delete $$; }
attributes(A) ::= attributes(B) COMMA attribute(C). {
    A = B;
    A->push_back(C);
}
attributes(A) ::= attribute(B). {
    std::list<Attribute*>* attributes =
        new std::list<Attribute*>();
    attributes->push_back(B);
    A = attributes;
}

%type attribute { Attribute* }
%destructor attribute { delete $$; }
attribute(A) ::=  NAME(Name) COLON type(Type). {
    A = new Attribute(*Name, Type);
}

%type type { Object::Type }
type(A) ::= TYPE_INT.    { A = Object::INT; }
type(A) ::= TYPE_FLOAT.  { A = Object::FLOAT; }
type(A) ::= TYPE_STRING. { A = Object::STRING; }
type(A) ::= TYPE_BLOB.   { A = Object::BLOB; }

%type operations { Operator* }
%destructor operations { delete $$; }
operations(Operations) ::= operations(PastOperations) operation(NewOperation). {
    LOG("Concat operations");
}
operations(Operations) ::= operation(NewOperation). {
    LOG("Parsed an operation: " << *(cpl_container->current_token_string));
    Operations = NewOperation;
}

%type operation { Operator* }
%destructor attribute { delete $$; }
operation ::= SELECT condition.    { LOG("SELECT"); }
operation ::= PROJECT fields.      { LOG("PROJECT"); }
operation ::= MEAN fields window.  { LOG("MEAN"); }
operation ::= SUM fields window.   { LOG("SUM"); }
operation ::= ELECT fields window. { LOG("ELECT"); }

window ::= RECENT window_value_tuple SLIDE window_value_tuple. { LOG("tuple base window"); }
window ::= RECENT window_value_time SLIDE window_value_time. { LOG("time base window"); }

window_value_tuple ::= INTEGER window_type_tuple.
window_value_tuple ::= INTEGER.
window_value_time ::= INTEGER window_type_time.
window_value_time ::= FLOAT window_type_time.

window_type_tuple ::= ROWS.
window_type_time ::= MSEC.
window_type_time ::= SEC.
window_type_time ::= MIN.
window_type_time ::= HOUR.
window_type_time ::= DAY.

fields ::= field.
fields ::= fields COMMA field.
field ::=  NAME DOT NAME.

condition ::= condition condition_conjunction condition_term. { LOG("condition"); }
condition ::= condition_term.  { LOG("condition (term)"); }

%type condition_term { Condition* }
condition_term ::= field(F1) bin_op(OP) field(F2). {
}
condition_term ::= field(F1) bin_op(OP) object(O1). {
}
condition_term ::= object(O1) bin_op(OP) field(F1). {
}
condition_term ::= NOT condition_term(OP). {
}

%type object { Object* }
%destructor attribute { delete $$; }
object(O) ::= STRING(S). { O = new Object(*S); }
object(O) ::= INTEGER(I). {
    std::stringstream ss;
    int value_int;
    ss << I;
    ss >> value_int;
    O = new Object(value_int);
}
object(O) ::= FLOAT(F). {
    std::stringstream ss;
    double value_float;
    ss << F;
    ss >> value_float;
    O = new Object(value_float);
}
object(O) ::= BLOB(B). { O = new Object(*B); }

condition_conjunction ::= AND.
condition_conjunction ::= OR.

%type bin_op { Comparator::Type }
bin_op(OP) ::= EQUAL(TK).              { OP = Lexer::token_to_comparator(TK);T }
bin_op(OP) ::= NOT_EQUAL(TK).          { OP = Lexer::token_to_comparator(TK);T }
bin_op(OP) ::= LESS_THAN(TK).          { OP = Lexer::token_to_comparator(TK);T }
bin_op(OP) ::= LESS_THAN_EQUAL(TK).    { OP = Lexer::token_to_comparator(TK);T }
bin_op(OP) ::= GREATER_THAN(TK).       { OP = Lexer::token_to_comparator(TK);T }
bin_op(OP) ::= GREATER_THAN_EQUAL(TK). { OP = Lexer::token_to_comparator(TK);T }
