// -*- c++ -*-

#ifndef CURRENTIA_QUERY_LEXER_H_
#define CURRENTIA_QUERY_LEXER_H_

#include <list>
#include <iostream>
#include <algorithm>
#include <functional>
#include <ctype.h>
#include <sstream>

#include "currentia/trait/non-copyable.h"

namespace currentia {
    class Lexer: private NonCopyable<Lexer> {
    public:
        typedef Lexer* ptr_t;

        enum Token {
            // {{{ DML --------------------------------
            SELECT,                 // "SELECT"
            FROM,                   // "FROM"
            WHERE,                  // "WHERE"
            // conjunctives
            AND,                    // "AND"
            OR,                     // "OR"
            // }}} DML --------------------------------

            // {{{ DDL --------------------------------
            CREATE,                 // "CREATE"
            STREAM,                 // "STREAM"
            TABLE,                  // "TABLE"
            // }}} DDL --------------------------------

            // identifier
            NAME,                   // [a-zA-Z_][a-zA-Z0-9_]*

            // string
            STRING,                 // "[\"]*"

            // numbers
            INTEGER,                // [0-9]
            FLOAT,                  // [0-9].[0-9]*

            // misc
            NOT,                    // NOT
            IN,                     // IN

            // window specification
            ROWS,                   // ROWS
            ADVANCE,                // ADVANCE
            LBRACKET,               // "["
            RBRACKET,               // "]"

            // other symbols
            COMMA,                  // ","
            DOT,                    // "."
            LPAREN,                 // "("
            RPAREN,                 // ")"

            // comparator
            EQUAL,                  // "="
            NOT_EQUAL,              // "!="
            LESS_THAN,              // "<"
            LESS_THAN_EQUAL,        // "<="
            GREATER_THAN,           // ">"
            GREATER_THAN_EQUAL,     // ">="

            // error
            UNKNOWN,
            EOS,                    //  End of Stream
        };

        static const int EOF_SIGN = -1;

        Lexer(std::istream* stream_ptr):
            stream_ptr_(stream_ptr),
            line_number_(1),
            cursor_point_(0) {
        }

        enum Token next_token() {
            while (true) {
                char next_char = peek_next_char_();

                switch (next_char) {
                case EOF_SIGN:
                    return EOS;
                case '"':
                    return rule_string_();
                case ',':
                    return rule_comma_();
                case '.':
                    return rule_dot_();
                case '(':
                    return rule_lparen_();
                case ')':
                    return rule_rparen_();
                case '[':
                    return rule_lbracket_();
                case ']':
                    return rule_rbracket_();
                case '=':
                    return rule_equal_();
                case '!':
                    return rule_not_equal_();
                case '>':
                    // > or >=
                    return rule_greater_than_or_equal_();
                case '<':
                    // < or <=
                    return rule_less_than_or_equal_();
                case '#':
                    consume_comment_();
                    continue;
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    // ignore
                    get_next_char_();
                    continue;
                }

                if (is_alphabet(next_char))
                    return rule_name_or_reserved_word_();

                if (is_number(next_char))
                    return rule_integer_or_float_();

                throw error_message_(std::string("Unknown character ") + next_char);
            }
        }

        static bool is_token_conjunctive(Token token) {
            return token == AND || token == OR;
        }

        static bool is_token_value(Token token) {
            return token == STRING || token == INTEGER || token == FLOAT;
        }

        static bool is_alphabet(int c) {
            return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
        }

        static bool is_number(int c) {
            return ('0' <= c && c <= '9');
        }

        static bool is_identifier_component(int c) {
            return is_alphabet(c) || is_number(c) || c == '_';
        }

        static std::string token_to_string(enum Token token) {
            switch (token) {
            case SELECT:
                return "SELECT";
            case FROM:
                return "FROM";
            case WHERE:
                return "WHERE";
            case AND:
                return "AND";
            case OR:
                return "OR";
            case NOT:
                return "NOT";
            case IN:
                return "IN";
            case ROWS:
                return "ROWS";
            case ADVANCE:
                return "ADVANCE";
            case CREATE:
                return "CREATE";
            case STREAM:
                return "STREAM";
            case TABLE:
                return "TABLE";
            case NAME:
                return "NAME";
            case STRING:
                return "STRING";
            case INTEGER:
                return "INTEGER";
            case FLOAT:
                return "FLOAT";
            case COMMA:
                return "COMMA";
            case DOT:
                return "DOT";
            case LPAREN:
                return "LPAREN";
            case RPAREN:
                return "RPAREN";
            case EQUAL:
                return "EQUAL";
            case NOT_EQUAL:
                return "NOT_EQUAL";
            case LESS_THAN:
                return "LESS_THAN";
            case LESS_THAN_EQUAL:
                return "LESS_THAN_EQUAL";
            case GREATER_THAN:
                return "GREATER_THAN";
            case GREATER_THAN_EQUAL:
                return "GREATER_THAN_EQUAL";
            case EOS:
                return "EOS";
            case UNKNOWN:
                return "UNKNOWN";
            default:
                return "MISSING_STRING_EXPRESSION";
            }
        }

        const std::string& get_latest_name() {
            return name_string_;
        }

        const std::string& get_latest_number() {
            return number_string_;
        }

        const std::string& get_latest_string() {
            return string_string_;
        }

        long get_current_line_number() {
            return line_number_;
        }

        long get_current_cursor_point() {
            return cursor_point_;
        }

        template <typename Stream>
        void print_error_point(Stream& stream = std::cout) {
            stream << current_line_ << std::endl;
            for (int i = 0; i < cursor_point_ - 1; ++i)
                stream << '_';
            stream << '^';
        }

    private:
        std::istream* stream_ptr_;

        long line_number_;
        long cursor_point_;
        std::string current_line_;

        typedef std::list<char> char_buffer_t;
        char_buffer_t next_char_buffer_;

        std::string error_message_(std::string message) {
            std::stringstream ss;
            ss << "line "<< line_number_ << ": " << message << std::endl;
            print_error_point(ss);
            return ss.str();
        }

        int dequeue_next_char_buffer_() {
            int next_char = next_char_buffer_.front();
            next_char_buffer_.pop_front();
            return next_char;
        }

        void enqueue_next_char_buffer_(int next_char) {
            next_char_buffer_.push_back(next_char);
        }

        int get_next_char_() {
            int next_char = next_char_buffer_.empty()
                            ? stream_ptr_->get()
                            : dequeue_next_char_buffer_();
            return std::toupper(next_char);
        }

        void do_read_one_char() {
            int next_char = get_next_char_();
            if (!stream_ptr_->good())
                next_char = EOF_SIGN;
            enqueue_next_char_buffer_(next_char);

            if (next_char == '\n') {
                current_line_.erase();
                line_number_++;
                cursor_point_ = 0;
            } else {
                current_line_.push_back(next_char);
                cursor_point_++;
            }
        }

        int peek_next_char_() {
            if (next_char_buffer_.empty())
                do_read_one_char();
            return next_char_buffer_.front();
        }

        bool is_char_available_() {
            return stream_ptr_->good() || !next_char_buffer_.empty();
        }

        // Rules

        enum Token rule_name_or_reserved_word_() {
            rule_name_();

            if (name_string_ == "CREATE")
                return CREATE;
            if (name_string_ == "STREAM")
                return STREAM;
            if (name_string_ == "TABLE")
                return TABLE;

            if (name_string_ == "SELECT")
                return SELECT;
            if (name_string_ == "FROM")
                return FROM;
            if (name_string_ == "WHERE")
                return WHERE;

            if (name_string_ == "AND")
                return AND;
            if (name_string_ == "OR")
                return OR;

            if (name_string_ == "NOT")
                return NOT;
            if (name_string_ == "IN")
                return IN;

            if (name_string_ == "ROWS" ||
                name_string_ == "TUPLES") // TUPLES is a synonym of ROWS
                return ROWS;

            if (name_string_ == "ADVANCE")
                return ADVANCE;

            return NAME;
        }

        std::string name_string_;
        enum Token rule_name_() {
            name_string_.clear();

            if (!is_alphabet(peek_next_char_()))
                throw error_message_("Expected alphabet");
            name_string_.push_back(get_next_char_());

            while (is_identifier_component(peek_next_char_()))
                name_string_.push_back(get_next_char_());

            return NAME;
        }

        std::string string_string_;
        enum Token rule_string_() {
            string_string_.clear();

            if (peek_next_char_() != '"')
                throw error_message_("Expected '\"'");
            get_next_char_();

            bool escaped = false;
            while (is_char_available_()) {
                // We do not need peeking
                char next_char = get_next_char_();

                if (escaped) {
                    // EOF_SIGN may be pushed into string_string_
                    // but it'll be discarded by unterminated string error
                    string_string_.push_back(next_char);
                    escaped = false;
                } else {
                    if (next_char == '"') {
                        return STRING;
                    } else if (next_char == '\\') {
                        escaped = true;
                    } else {
                        string_string_.push_back(next_char);
                    }
                }
            }

            throw error_message_("Unterminated string");
        }

        std::string number_string_;
        enum Token rule_integer_or_float_() {
            number_string_.clear();

            // TODO: allow beginning with '0'?
            if (!is_number(peek_next_char_()))
                throw error_message_("Expected number");
            number_string_.push_back(get_next_char_());

            while (is_number(peek_next_char_()))
                number_string_.push_back(get_next_char_());

            if (peek_next_char_() != '.')
                return INTEGER;
            number_string_.push_back(get_next_char_());
            while (is_number(peek_next_char_()))
                number_string_.push_back(get_next_char_());

            return FLOAT;
        }

        enum Token rule_comma_() {
            get_next_char_();
            return COMMA;
        }

        enum Token rule_dot_() {
            get_next_char_();
            return DOT;
        }

        enum Token rule_lparen_() {
            get_next_char_();
            return LPAREN;
        }

        enum Token rule_rparen_() {
            get_next_char_();
            return RPAREN;
        }

        enum Token rule_lbracket_() {
            get_next_char_();
            return LBRACKET;
        }

        enum Token rule_rbracket_() {
            get_next_char_();
            return RBRACKET;
        }

        enum Token rule_equal_() {
            get_next_char_();
            return EQUAL;
        }

        enum Token rule_not_equal_() {
            get_next_char_();
            if (peek_next_char_() != '=')
                throw error_message_("Expected '='");
            get_next_char_();
            return NOT_EQUAL;
        }

        enum Token rule_greater_than_or_equal_() {
            get_next_char_();
            if (peek_next_char_() != '=')
                return GREATER_THAN;
            get_next_char_();
            return GREATER_THAN_EQUAL;
        }

        enum Token rule_less_than_or_equal_() {
            get_next_char_();
            if (peek_next_char_() != '=')
                return LESS_THAN;
            get_next_char_();
            return LESS_THAN_EQUAL;
        }

        void consume_comment_() {
            if (get_next_char_() != '#')
                throw error_message_("Expected '#'");

            while (is_char_available_() && peek_next_char_() != '\n')
                get_next_char_();
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_LEXER_H_  */
