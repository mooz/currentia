%token_type { std::string* }

%extra_argument { CPLQueryContainer* query_container }

%name CPLParse
%token_prefix TOKEN_

%include {
#include <cstdio>
#include <iostream>
#include <string>
#include <assert.h>

// No error recovery
#define YYNOERRORRECOVERY 1

#include "currentia/query/cpl.h"
#include "currentia/query/cpl-parser.h"
}

%syntax_error {
    query_container->state = CPLQueryContainer::ERROR;
}

%parse_failure {
    std::cerr << "Failed to parse input" << std::endl;
}

%parse_accept {
    std::cerr << "Parse accepted" << std::endl;
}

%token_destructor {
    delete $$;
}

plan ::= statements.
plan ::= UNKNOWN. {
    std::cerr << "Unknown token" << std::endl;
}

statements ::= statements statement.
statements ::= statement.

statement ::= define_relation(RelationDeclaration). {
    RelationDeclaration->declare(query_container);
    delete RelationDeclaration;
}
statement ::= define_stream(StreamDeclaration). {
    StreamDeclaration->declare(query_container);
    delete StreamDeclaration;
}

// ------------------------------------------------------------
// Relation definition
// ------------------------------------------------------------

%type define_relation { CPLRelationDeclaration* }
%destructor define_relation { delete $$; }
define_relation(A) ::= RELATION NAME(RelationName) LPAREN attributes(Attributes) RPAREN. {
    A = new CPLRelationDeclaration(*RelationName, Attributes);
    delete RelationName;
}

// ------------------------------------------------------------
// Stream definition
// ------------------------------------------------------------

%type define_stream { CPLStreamDeclaration* }
%destructor define_stream { delete $$; }
define_stream(A) ::= new_stream(NewStream). {
    A = NewStream;
}
define_stream(A) ::= derived_stream(DerivedStream). {
    A = DerivedStream;
}

// ------------------------------------------------------------
// Stream definition / New Stream
// ------------------------------------------------------------

%type new_stream { CPLNewStream* }
%destructor new_stream { delete $$; }
new_stream(A) ::= STREAM NAME(StreamName) LPAREN attributes(Attributes) RPAREN. {
    A = new CPLNewStream(*StreamName, Attributes);
}

// ------------------------------------------------------------
// Stream definition / Derived Stream
// Create a stream from a sequence of operation
// ------------------------------------------------------------

%type derived_stream { CPLDerivedStream* }
%destructor derived_stream { delete $$; }
derived_stream(A) ::= STREAM NAME(NewStreamName) FROM derived_from(DeriveInfo) operations_block(Operations). {
    DeriveInfo->stream_name = *NewStreamName;
    DeriveInfo->operations_ptr = Operations;
    A = DeriveInfo;
}

%type derived_from { CPLDerivedStream* }
%destructor derived_from { delete $$; }
derived_from(A) ::= stream_subtree(LeftRoot) window(W1) COMMA stream_subtree(RightRoot) window(W2) WHERE condition(Condition). {
    A = new CPLJoinedStream(LeftRoot, *W1, RightRoot, *W2, Condition);
}
derived_from(A) ::= stream_subtree(Root). {
    A = new CPLSingleStream(Root);
}

%type stream_subtree { Operator::ptr_t* }
%destructor stream_subtree { delete $$; }
// Since lemon retain rule values in union, we cannot use
// Oparator::ptr_t, which have a constructor. And if we get raw
// pointer Oparator* by shared_ptr.get(), it breaks the memory
// management of shared_ptr. So, as a workaround, we use dynamically
// allocated shared_ptr to share the reference counter.
stream_subtree(A) ::= NAME(StreamName). {
    A = new Operator::ptr_t(query_container->get_root_operator_by_stream_name(*StreamName));
}
stream_subtree(A) ::= derived_anonymous_stream(DerivedInfo). {
    A = new Operator::ptr_t(DerivedInfo->get_operator_tree(query_container));
}

%type derived_anonymous_stream { CPLDerivedStream* }
%destructor derived_anonymous_stream { delete $$; }
derived_anonymous_stream(A) ::= LPAREN STREAM FROM derived_from(DeriveInfo) operations_block(Operations) RPAREN. {
    DeriveInfo->operations_ptr = Operations;
    A = DeriveInfo;
}

%type operations_block { std::list<CPLOperationInfo*>* }
operations_block(A) ::= LBRACE operations(Operations) RBRACE. {
    A = Operations;
}
operations_block(A) ::= LBRACE RBRACE. {
    A = NULL;
}
operations_block(A) ::= . {
    A = NULL;
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
%destructor operation { delete $$; }
operation(A) ::= SELECT condition(C).       { A = new CPLOperationInfo(CPLOperationInfo::SELECT, C); }
operation(A) ::= PROJECT fields(F).         { A = new CPLOperationInfo(CPLOperationInfo::PROJECT, F); }
operation(A) ::= MEAN fields(F) window(W).  { A = new CPLOperationInfo(CPLOperationInfo::MEAN, F, W); }
operation(A) ::= SUM fields(F) window(W).   { A = new CPLOperationInfo(CPLOperationInfo::SUM, F, W); }
operation(A) ::= ELECT fields(F) window(W). { A = new CPLOperationInfo(CPLOperationInfo::ELECT, F, W); }
operation(A) ::= COMBINE NAME(N) WHERE condition(C). {
    A = new CPLOperationInfo(CPLOperationInfo::COMBINE, *N, C);
}

// ------------------------------------------------------------
// Window
// ------------------------------------------------------------

%type window { Window* }
%destructor window { delete $$; }
window(A) ::= LBRACKET window_info(Info) RBRACKET. {
    A = Info;
}

%type window_info { Window* }
%destructor window_info { delete $$; }
window_info(A) ::= RECENT window_value_tuple(Width) SLIDE window_value_tuple(Slide). {
    A = new Window(Width, Slide, Window::TUPLE_BASE);
}
window_info(A) ::= RECENT window_value_tuple(Width). {
    A = new Window(Width, Width, Window::TUPLE_BASE);
}
window_info(A) ::= window_value_tuple(Width). {
    A = new Window(Width, Width, Window::TUPLE_BASE);
}
window_info(A) ::= RECENT window_value_time(Width) SLIDE window_value_time(Slide). {
    A = new Window(Width, Slide, Window::TIME_BASE);
}
window_info(A) ::= RECENT window_value_time(Width). {
    A = new Window(Width, Width, Window::TIME_BASE);
}
window_info(A) ::= window_value_time(Width). {
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
condition_term(A) ::= condition_term_attribute(C). {
    A = C;
}
condition_term(A) ::= condition_term_constant(C). {
    A = C;
}
condition_term(A) ::= NOT condition_term(C). {
    A = C;
    C->negate();
}
%type condition_term_attribute { ConditionAttributeComparator* }
%destructor condition_term_attribute { delete $$; }
condition_term_attribute(A) ::= field(F1) bin_op(OP) field(F2). {
    A = new ConditionAttributeComparator(F1->field_name, OP, F2->field_name);
}
%type condition_term_constant { ConditionConstantComparator* }
%destructor condition_term_constant { delete $$; }
condition_term_constant(A) ::= field(F1) bin_op(OP) object(O1). {
    A = new ConditionConstantComparator(F1->field_name, OP, *O1);
}
condition_term_constant(A) ::= object(O1) bin_op(OP) field(F1). {
    A = new ConditionConstantComparator(F1->field_name, OP, *O1);
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
    O = new Object(CPLLexer::parse_int(*I));
}
object(O) ::= FLOAT(F). {
    O = new Object(CPLLexer::parse_float(*F));
}
object(O) ::= BLOB(B). { O = new Object(*B); }

// ------------------------------------------------------------
// Number
// ------------------------------------------------------------

%type number { double }
number(N) ::= INTEGER(I). {
    N = static_cast<double>(CPLLexer::parse_int(*I));
}
number(N) ::= FLOAT(F). {
    N = CPLLexer::parse_float(*F);
}
