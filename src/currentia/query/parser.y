%token_type { std::string* }

%extra_argument { currentia::CPLContainer* cpl_container }

%name CPLParse
%token_prefix TOKEN_

%include {
#include <iostream>
#include <assert.h>
#include "cpl.h"
#include "lemon.h"

    enum Token {
        CURRENTIA_DEFINE_TOKEN_LIST(CURRENTIA_DEFINE_ENUM)
    };
}

%syntax_error {
    std::cerr << "Syntax error!" << std::endl;
}

plan ::= statements EOS.
plan ::= UNKNOWN. {
    std::cerr << "Unknown token!" << std::endl;
}

statements ::= statements statement.
statements ::= statement.

statement ::= define_relation.
statement ::= define_stream.

define_relation ::= RELATION LPAREN attributes(A) RPAREN. {
    // TODO (create relation from attribute list)
}

define_stream ::= STREAM LPAREN attributes RPAREN.
define_stream ::= STREAM LBRACE operations RBRACE.

attributes ::= attribute.
attributes ::= attributes COMMA attribute.
attribute ::=  NAME COLON type.

type ::= TYPE_INT.
type ::= TYPE_FLOAT.
type ::= TYPE_STRING.
type ::= TYPE_BLOB.

operations ::= operations operation.
operations ::= operation.

operation ::= SELECT condition.
operation ::= PROJECT fields.
operation ::= MEAN fields window.
operation ::= SUM fields window.
operation ::= ELECT fields window.
operation ::= JOIN NAME(A) WITH NAME(B) WHERE condition.

window ::= WIDTH window_value_tuple SLIDE window_value_tuple.
window ::= WIDTH window_value_time SLIDE window_value_time.

window_value_tuple ::= INTEGER window_type_tuple.
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
field ::=  NAME DOT type.

condition ::= condition_term.
condition ::= condition condition_conjunction condition_term.

condition_term ::= field(A) bin_op(OP) field(B).
condition_term ::= field(A) bin_op(OP) object.
condition_term ::= object bin_op(OP) field(B).
condition_term ::= NOT condition_term.

object ::= STRING.
object ::= INTEGER.
object ::= FLOAT.
object ::= BLOB.

condition_conjunction ::= AND.
condition_conjunction ::= OR.

bin_op ::= EQUAL.
bin_op ::= NOT_EQUAL.
bin_op ::= LESS_THAN.
bin_op ::= LESS_THAN_EQUAL.
bin_op ::= GREATER_THAN.
bin_op ::= GREATER_THAN_EQUAL.
