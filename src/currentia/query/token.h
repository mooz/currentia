#ifndef currentia_QUERY_TOKEN_H_
#define currentia_QUERY_TOKEN_H_

/* #define CURRENTIA_DEFINE_MACRO(TOKEN, AS_STRING) #define TOKEN */

#define CURRENTIA_DEFINE_ENUM(TOKEN, AS_STRING) TOKEN,

#define CURRENTIA_DEFINE_SWITCH_STRING(TOKEN, AS_STRING)        \
    case TOKEN: return AS_STRING;

/* AS_STRING is redundant? Use ##TOKEN instead? */
#define CURRENTIA_DEFINE_TOKEN_LIST(DEFINE)                     \
    DEFINE(TOKEN_SELECT, "SELECT")                              \
    DEFINE(TOKEN_PROJECT, "PROJECT")                            \
    DEFINE(TOKEN_JOIN, "JOIN")                                  \
                                                                \
    DEFINE(TOKEN_FROM, "FROM")                                  \
    DEFINE(TOKEN_WHERE, "WHERE")                                \
    /* conjunctives */                                          \
    DEFINE(TOKEN_AND, "AND")                                    \
    DEFINE(TOKEN_OR, "OR")                                      \
    /* }}} DML -------------------------------- */              \
                                                                \
    DEFINE(TOKEN_SUM, "SUM")                                    \
    DEFINE(TOKEN_MEAN, "MEAN")                                  \
    DEFINE(TOKEN_ELECT, "ELECT")                                \
                                                                \
    /* {{{ DDL -------------------------------- */              \
    DEFINE(TOKEN_CREATE, "CREATE")                              \
    DEFINE(TOKEN_STREAM, "STREAM")                              \
    DEFINE(TOKEN_TABLE, "TABLE")                                \
    DEFINE(TOKEN_RELATION, "RELATION")                          \
    /* }}} DDL -------------------------------- */              \
                                                                \
    /* identifier */                                            \
    DEFINE(TOKEN_NAME, "NAME")                                  \
                                                                \
    /* string */                                                \
    DEFINE(TOKEN_STRING, "STRING")                              \
                                                                \
    /* numbers */                                               \
    DEFINE(TOKEN_INTEGER, "INTEGER")                            \
    DEFINE(TOKEN_FLOAT, "FLOAT")                                \
                                                                \
    /* misc */                                                  \
    DEFINE(TOKEN_NOT, "NOT")                                    \
    DEFINE(TOKEN_IN, "IN")                                      \
                                                                \
    /* window specification */                                  \
    DEFINE(TOKEN_ROWS, "ROWS")                                  \
    DEFINE(TOKEN_MSEC, "MSEC")                                  \
    DEFINE(TOKEN_SEC, "SEC")                                    \
    DEFINE(TOKEN_MIN, "MIN")                                    \
    DEFINE(TOKEN_HOUR, "HOUR")                                  \
    DEFINE(TOKEN_DAY, "DAY")                                    \
    DEFINE(TOKEN_ADVANCE, "ADVANCE")                            \
    DEFINE(TOKEN_LBRACKET, "LBRACKET")                          \
    DEFINE(TOKEN_RBRACKET, "RBRACKET")                          \
                                                                \
    DEFINE(TOKEN_LBRACE, "LBRACE")                              \
    DEFINE(TOKEN_RBRACE, "RBRACE")                              \
                                                                \
    /* other symbols */                                         \
    DEFINE(TOKEN_COMMA, "COMMA")                                \
    DEFINE(TOKEN_DOT, "DOT")                                    \
    DEFINE(TOKEN_LPAREN, "LPAREN")                              \
    DEFINE(TOKEN_RPAREN, "RPAREN")                              \
                                                                \
    /* comparator */                                            \
    DEFINE(TOKEN_EQUAL, "EQUAL")                                \
    DEFINE(TOKEN_NOT_EQUAL, "NOT_EQUAL")                        \
    DEFINE(TOKEN_LESS_THAN, "LESS_THAN")                        \
    DEFINE(TOKEN_LESS_THAN_EQUAL, "LESS_THAN_EQUAL")            \
    DEFINE(TOKEN_GREATER_THAN, "GREATER_THAN")                  \
    DEFINE(TOKEN_GREATER_THAN_EQUAL, "GREATER_THAN_EQUAL")      \
                                                                \
    DEFINE(TOKEN_COUNT, "COUNT")                                \
    DEFINE(TOKEN_SLIDE, "SLIDE")                                \
    DEFINE(TOKEN_WITH, "WITH")                                  \
    DEFINE(TOKEN_COLON, "COLON")                                \
                                                                \
    DEFINE(TOKEN_TYPE_INT, "TYPE_INT")                          \
    DEFINE(TOKEN_TYPE_FLOAT, "TYPE_FLOAT")                      \
    DEFINE(TOKEN_TYPE_STRING, "TYPE_STRING")                    \
    DEFINE(TOKEN_TYPE_BLOB, "TYPE_BLOB")                        \
                                                                \
    /* error */                                                 \
    DEFINE(TOKEN_UNKNOWN, "UNKNOWN")                            \
    DEFINE(TOKEN_EOS, "EOS")

#endif  /* ! currentia_QUERY_TOKEN_H_ */