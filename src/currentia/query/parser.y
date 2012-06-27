%token_type { std::string* }

%extra_argument { CPLContainer* cpl_container }

%name CPLParse
%token_prefix TOKEN_

%include {
#include <iostream>
#include <string>
#include <assert.h>

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

define_stream ::= new_stream(NewStream). {
    std::cout << "New Stream [" << NewStream->stream_name << "] is defined." << std::endl;
    // Define new stream into cpl_container;
}
define_stream ::= derived_stream(DerivedStream). {
    std::cout << "Derived Stream [" << DerivedStream->stream_name << "] is defined." << std::endl;
    // Define new stream into cpl_container;
}

// ------------------------------------------------------------
// Stream definition / New Stream
// ------------------------------------------------------------

%type new_stream { CPLNewStream* }
%destructor new_stream { delete $$; }
new_stream(A) ::= STREAM NAME(StreamName) LPAREN attributes(Attributes) RPAREN. {
    A = new CPLNewStream();
    A->stream_name = *StreamName;
    A->attributes_ptr = Attributes;
}

// ------------------------------------------------------------
// Stream definition / Derived Stream
// Create a stream from a sequence of operation
// ------------------------------------------------------------

%type derived_stream { CPLDerivedStream* }
%destructor derived_stream { delete $$; }
derived_stream(A) ::= STREAM NAME(NewStreamName) FROM derived_from(DeriveInfo) LBRACE operations(Operations) RBRACE. {
    LOG("OperationCount => " << Operations->size());
    DeriveInfo->stream_name = *NewStreamName;
    A = DeriveInfo;
}
derived_stream(A) ::= STREAM NAME(NewStreamName) FROM derived_from(DeriveInfo) LBRACE RBRACE. {
    DeriveInfo->stream_name = *NewStreamName;
    A = DeriveInfo;
}
derived_stream(A) ::= STREAM NAME(NewStreamName) FROM derived_from(DeriveInfo). {
    DeriveInfo->stream_name = *NewStreamName;
    A = DeriveInfo;
}

// CPLDerivedStream
%type derived_from { CPLDerivedStream* }
%destructor derived_from { delete $$; }
derived_from(A) ::= NAME(Left) COMMA NAME(Right) LBRACKET condition(Condition) RBRACKET. {
    A = new CPLDerivedStream(*Left, *Right, Condition);
}
derived_from(A) ::= NAME(Single). {
    A = new CPLDerivedStream(*Single);
}

// ------------------------------------------------------------
// Operation (Operator)
// ------------------------------------------------------------

%type operations { std::list<CPLOperationInfo*>* }
%destructor operations { delete $$; }
operations(Operations) ::= operations(PastOperations) operation(NewOperation). {
    PastOperations->push_back(NewOperation);
    Operations = PastOperations;
}
operations(Operations) ::= operation(NewOperation). {
    Operations = new std::list<CPLOperationInfo*>();
    Operations->push_back(NewOperation);
}

%type operation { CPLOperationInfo* }
%destructor attribute { delete $$; }
operation(A) ::= SELECT condition(C).       { A = new CPLOperationInfo(CPLOperationInfo::SELECT, C); }
operation(A) ::= PROJECT fields(F).         { A = new CPLOperationInfo(CPLOperationInfo::PROJECT, F); }
operation(A) ::= MEAN fields(F) window(W).  { A = new CPLOperationInfo(CPLOperationInfo::MEAN, F, W); }
operation(A) ::= SUM fields(F) window(W).   { A = new CPLOperationInfo(CPLOperationInfo::SUM, F, W); }
operation(A) ::= ELECT fields(F) window(W). { A = new CPLOperationInfo(CPLOperationInfo::ELECT, F, W); }

// ------------------------------------------------------------
// Window
// ------------------------------------------------------------

%type window { Window* }
%destructor window { delete $$; }
window(A) ::= RECENT window_value_tuple(Width) SLIDE window_value_tuple(Slide). {
    A = new Window(Width, Slide, Window::TUPLE_BASE);
}
window(A) ::= RECENT window_value_tuple(Width). {
    A = new Window(Width, Width, Window::TUPLE_BASE);
}
window(A) ::= RECENT window_value_time(Width) SLIDE window_value_time(Slide). {
    A = new Window(Width, Slide, Window::TIME_BASE);
}
window(A) ::= RECENT window_value_time(Width). {
    A = new Window(Width, Width, Window::TIME_BASE);
}
 
// --- tuple-base window value
%type window_value_tuple { long }
window_value_tuple(A) ::= number(N) window_type_tuple. {
    A = static_cast<long>(N);
}
window_value_tuple(A) ::= number(N). {
    A = static_cast<long>(N);
}
window_type_tuple ::= ROWS.

// --- time-base window value
%type window_value_time { long }
window_value_time(A) ::= number(N) window_type_time(T). {
    A = static_cast<long>(N * T);
}
%type window_type_time { double }
window_type_time(T) ::= MSEC. { T = 1.0; }
window_type_time(T) ::= SEC.  { T = 1000.0; }
window_type_time(T) ::= MIN.  { T = 1000.0 * 60; }
window_type_time(T) ::= HOUR. { T = 1000.0 * 60 * 60; }
window_type_time(T) ::= DAY.  { T = 1000.0 * 60 * 60 * 24; }

// ------------------------------------------------------------
// Field (stream.field1)
// ------------------------------------------------------------

%type fields { std::list<CPLField*>* }
%destructor fields { delete $$; }
fields(A) ::= fields(Fields) COMMA field(Field). {
    A = Fields;
    Fields->push_back(Field);
}
fields(A) ::= field(Field). {
    A = new std::list<CPLField*>();
    A->push_back(Field);
}

%type field { CPLField* }
%destructor field { delete $$; }
field(A) ::=  NAME(BaseName) DOT NAME(FieldName). {
    A = new CPLField(*BaseName, *FieldName);
}

// ------------------------------------------------------------
// Condition
// ------------------------------------------------------------

%type condition { Condition* }
%destructor condition { delete $$; }
condition(A) ::= condition(C1) condition_conjunction(CC) condition_term(C2). {
    A = new ConditionConjunctive(Condition::ptr_t(C1), Condition::ptr_t(C2), CC);
}
condition(A) ::= condition_term(T).  { A = T; }

%type condition_conjunction { ConditionConjunctive::Type }
condition_conjunction(A) ::= AND. { A = ConditionConjunctive::AND; }
condition_conjunction(A) ::= OR.  { A = ConditionConjunctive::OR; }

%type condition_term { Condition* }
%destructor condition_term { delete $$; }
condition_term(A) ::= field(F1) bin_op(OP) field(F2). {
    A = new ConditionAttributeComparator(F1->field_name, OP, F2->field_name);
}
condition_term(A) ::= field(F1) bin_op(OP) object(O1). {
    A = new ConditionConstantComparator(F1->field_name, OP, *O1);
}
condition_term(A) ::= object(O1) bin_op(OP) field(F1). {
    A = new ConditionConstantComparator(F1->field_name, OP, *O1);
}
condition_term(A) ::= NOT condition_term(C). {
    A = C;
    C->negate();
}

// ------------------------------------------------------------
// Comparator
// ------------------------------------------------------------

%type bin_op { Comparator::Type }
bin_op(OP) ::= EQUAL.              { OP = Comparator::EQUAL; }
bin_op(OP) ::= NOT_EQUAL.          { OP = Comparator::NOT_EQUAL; }
bin_op(OP) ::= LESS_THAN.          { OP = Comparator::LESS_THAN; }
bin_op(OP) ::= LESS_THAN_EQUAL.    { OP = Comparator::LESS_THAN_EQUAL; }
bin_op(OP) ::= GREATER_THAN.       { OP = Comparator::GREATER_THAN; }
bin_op(OP) ::= GREATER_THAN_EQUAL. { OP = Comparator::GREATER_THAN_EQUAL; }

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

// ------------------------------------------------------------
// Object Types
// ------------------------------------------------------------

%type type { Object::Type }
type(A) ::= TYPE_INT.    { A = Object::INT; }
type(A) ::= TYPE_FLOAT.  { A = Object::FLOAT; }
type(A) ::= TYPE_STRING. { A = Object::STRING; }
type(A) ::= TYPE_BLOB.   { A = Object::BLOB; }

// ------------------------------------------------------------
// Object
// ------------------------------------------------------------

%type object { Object* }
%destructor attribute { delete $$; }
object(O) ::= STRING(S). { O = new Object(*S); }
object(O) ::= INTEGER(I). {
    O = new Object(Lexer::parse_int(*I));
}
object(O) ::= FLOAT(F). {
    O = new Object(Lexer::parse_float(*F));
}
object(O) ::= BLOB(B). { O = new Object(*B); }

// ------------------------------------------------------------
// Number
// ------------------------------------------------------------

%type number { double }
number(N) ::= INTEGER(I). {
    N = static_cast<double>(Lexer::parse_int(*I));
}
number(N) ::= FLOAT(F). {
    N = Lexer::parse_float(*F);
}
