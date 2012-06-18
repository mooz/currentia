// -*- c++ -*-

// re2c --case-insensitive -i -o lex.cc lex.re

#ifndef CURRENTIA_QUERY_LEXER_H_
#define CURRENTIA_QUERY_LEXER_H_

#include <list>
#include <iostream>
#include <algorithm>
#include <functional>
#include <sstream>

#include <ctype.h>
#include <string.h>

#include "currentia/trait/non-copyable.h"

/*!re2c
  re2c:define:YYCTYPE = "char";
  re2c:define:YYCURSOR = yy_cursor_;
  re2c:define:YYMARKER = yy_marker_;
  re2c:define:YYLIMIT = yy_limit_;
  re2c:define:YYFILL:naked = 1;
  re2c:define:YYFILL@len = #;
  re2c:define:YYFILL = "if (!fill(#)) { return TOKEN_EOS; }";
  re2c:yyfill:enable = 1;
  re2c:indent:top = 3;
  re2c:indent:string="    ";
*/

#define dump(x) \
        std::cout << #x << " => [" << (x) << "]" << std::endl

namespace currentia {
    class Lexer : private NonCopyable<Lexer> {
    private:
        std::istream* ifs_ptr_;

        char* buffer_;
        char* yy_cursor_;
        char* yy_marker_;
        char* yy_limit_;
        char* token_begin_;
        long buffer_size_;
        long line_number_;

    public:
        typedef Lexer* ptr_t;

        enum Token {
            // {{{ DML --------------------------------
            TOKEN_SELECT,                 // "SELECT"
            TOKEN_FROM,                   // "FROM"
            TOKEN_WHERE,                  // "WHERE"
            // conjunctives
            TOKEN_AND,                    // "AND"
            TOKEN_OR,                     // "OR"
            // }}} DML --------------------------------

            // {{{ DDL --------------------------------
            TOKEN_CREATE,                 // "CREATE"
            TOKEN_STREAM,                 // "STREAM"
            TOKEN_TABLE,                  // "TABLE"
            // }}} DDL --------------------------------

            // identifier
            TOKEN_NAME,                   // [a-zA-Z_][a-zA-Z0-9_]*

            // string
            TOKEN_STRING,                 // "[\"]*"

            // numbers
            TOKEN_INTEGER,                // [1-9][0-9]*
            TOKEN_FLOAT,                  // [0-9].[0-9]*

            // misc
            TOKEN_NOT,                    // NOT
            TOKEN_IN,                     // IN

            // window specification
            TOKEN_ROWS,                   // ROWS
            TOKEN_MSEC,                   // MSEC
            TOKEN_SEC,                    // SEC
            TOKEN_MIN,                    // MIN
            TOKEN_HOUR,                   // HOUR
            TOKEN_DAY,                    // DAY
            TOKEN_ADVANCE,                // ADVANCE
            TOKEN_LBRACKET,               // "["
            TOKEN_RBRACKET,               // "]"

            // other symbols
            TOKEN_COMMA,                  // ","
            TOKEN_DOT,                    // "."
            TOKEN_LPAREN,                 // "("
            TOKEN_RPAREN,                 // ")"

            // comparator
            TOKEN_EQUAL,                  // "="
            TOKEN_NOT_EQUAL,              // "!="
            TOKEN_LESS_THAN,              // "<"
            TOKEN_LESS_THAN_EQUAL,        // "<="
            TOKEN_GREATER_THAN,           // ">"
            TOKEN_GREATER_THAN_EQUAL,     // ">="

            // error
            TOKEN_UNKNOWN,
            TOKEN_EOS,                    //  End of Stream
        };

        Lexer(std::istream* ifs_ptr, long buffer_init_size = 1024):
            ifs_ptr_(ifs_ptr),
            buffer_size_(buffer_init_size),
            line_number_(1) {
            // Allocate
            buffer_ = reinterpret_cast<char*>(malloc(sizeof(char) * buffer_size_));
            // Init
            yy_cursor_ = yy_marker_ = yy_limit_ = token_begin_ = buffer_;
        }

        ~Lexer() {
            free(buffer_);
        }

        inline
        int get_buffer_used_size() {
            return yy_limit_ - buffer_;
        }

        inline
        int get_buffer_allocated_size() {
            return buffer_size_;
        }

        bool fill(int n) {
            if (ifs_ptr_->eof() && (yy_limit_ - yy_cursor_) <= 0)
                return false;   // EOS

            int cursor_offset = yy_cursor_ - buffer_;
            int remained_characters_count = yy_limit_ - yy_cursor_;

            if (remained_characters_count + n >= buffer_size_) {
                // Extend buffer
                int yy_marker_offset = yy_marker_ - buffer_;
                int yy_limit_offset = yy_limit_ - buffer_;
                buffer_ = reinterpret_cast<char*>(realloc(buffer_, buffer_size_ *= 2));
                // Refresh Re-allocated memory
                yy_marker_ = buffer_ + yy_marker_offset;
                yy_limit_ = yy_limit_ + yy_limit_offset;
            } else if (remained_characters_count > 0) {
                // Shift remained characters in the buffer_ to head of the buffer_
                memmove(buffer_, yy_cursor_, sizeof(char) * remained_characters_count);
            }

            // Adjust position (shift)
            yy_cursor_ = token_begin_ = buffer_;
            yy_marker_ -= cursor_offset;
            yy_limit_ -= cursor_offset;

            // This line is very important (fill buffer with EOF)
            memset(yy_limit_, '\0', sizeof(char) * (buffer_size_ - remained_characters_count));

            // Read characters from input stream
            int read_size = buffer_size_ - remained_characters_count;
            ifs_ptr_->read(yy_limit_, read_size);
            yy_limit_ += ifs_ptr_->gcount();
            // Dirty hack (for stringstream which doesn't give us a EOF character)
            if (ifs_ptr_->gcount() == 0 && *(yy_limit_ -1) != '\0') {
                *yy_limit_ = '\0';
                ++yy_limit_;
            }

            return true;
        }

        std::string get_token_text() {
            return std::string(token_begin_, token_begin_ + get_token_length());
        }
        int get_token_length() {
            return yy_cursor_ - token_begin_;
        }

        enum Token get_next_token() {
        start:
            token_begin_ = yy_cursor_;

            /*!re2c

              IDENTIFIER  = [a-zA-Z_][a-zA-Z_0-9]*;
              WHITESPACES = [ \t\v\f]+;
              NEWLINE     = [\r]?|[\n];
              COMMENT     = [#] .* NEWLINE;
              DIGIT       = [0-9];
              STRING      = ["] ([\].|[^"])* ["];

              COMMENT           { goto start; }
              WHITESPACES       { goto start; }
              NEWLINE           { line_number_++; goto start; }

              "SELECT"          { return TOKEN_SELECT; }
              "FROM"            { return TOKEN_FROM; }
              "WHERE"           { return TOKEN_WHERE; }
              "AND"             { return TOKEN_AND; }
              "OR"              { return TOKEN_OR; }

              "CREATE"          { return TOKEN_CREATE; }
              "STREAM"          { return TOKEN_STREAM; }
              "TABLE"           { return TOKEN_TABLE; }

              "NOT"             { return TOKEN_NOT; }
              "IN"              { return TOKEN_IN; }

              "ROWS"            { return TOKEN_ROWS; }
              "MSEC"            { return TOKEN_MSEC; }
              "SEC"             { return TOKEN_SEC; }
              "MIN"             { return TOKEN_MIN; }
              "HOUR"            { return TOKEN_HOUR; }
              "DAY"             { return TOKEN_DAY; }
              "ADVANCE"         { return TOKEN_ADVANCE; }

              IDENTIFIER        { return TOKEN_NAME; }

              STRING            { return TOKEN_STRING; }
              DIGIT+ [.] DIGIT* { return TOKEN_FLOAT; }
              DIGIT             { return TOKEN_INTEGER; }

              "["               { return TOKEN_LBRACKET; }
              "]"               { return TOKEN_RBRACKET; }
              ","               { return TOKEN_COMMA; }
              "."               { return TOKEN_DOT; }
              "("               { return TOKEN_LPAREN; }
              ")"               { return TOKEN_RPAREN; }
              "="               { return TOKEN_EQUAL; }
              "!="              { return TOKEN_NOT_EQUAL; }
              "<"               { return TOKEN_LESS_THAN; }
              "<="              { return TOKEN_LESS_THAN_EQUAL; }
              ">"               { return TOKEN_GREATER_THAN; }
              ">="              { return TOKEN_GREATER_THAN_EQUAL; }

              [\x00]            { return TOKEN_EOS; }
              .                 { return TOKEN_UNKNOWN; }

            */
        }

        static bool is_token_conjunctive(Token token) {
            return token == TOKEN_AND || token == TOKEN_OR;
        }

        static bool is_token_value(Token token) {
            return token == TOKEN_STRING || token == TOKEN_INTEGER || token == TOKEN_FLOAT;
        }

        static std::string token_to_string(enum Token token) {
            switch (token) {
            case TOKEN_SELECT:
                return "SELECT";
            case TOKEN_FROM:
                return "FROM";
            case TOKEN_WHERE:
                return "WHERE";
            case TOKEN_AND:
                return "AND";
            case TOKEN_OR:
                return "OR";
            case TOKEN_NOT:
                return "NOT";
            case TOKEN_IN:
                return "IN";
            case TOKEN_ROWS:
                return "ROWS";
            case TOKEN_MSEC:
                return "MSEC";
            case TOKEN_SEC:
                return "SEC";
            case TOKEN_MIN:
                return "MIN";
            case TOKEN_HOUR:
                return "HOUR";
            case TOKEN_DAY:
                return "DAY";
            case TOKEN_ADVANCE:
                return "ADVANCE";
            case TOKEN_CREATE:
                return "CREATE";
            case TOKEN_STREAM:
                return "STREAM";
            case TOKEN_TABLE:
                return "TABLE";
            case TOKEN_NAME:
                return "NAME";
            case TOKEN_STRING:
                return "STRING";
            case TOKEN_INTEGER:
                return "INTEGER";
            case TOKEN_FLOAT:
                return "FLOAT";
            case TOKEN_COMMA:
                return "COMMA";
            case TOKEN_DOT:
                return "DOT";
            case TOKEN_LPAREN:
                return "LPAREN";
            case TOKEN_RPAREN:
                return "RPAREN";
            case TOKEN_EQUAL:
                return "EQUAL";
            case TOKEN_NOT_EQUAL:
                return "NOT_EQUAL";
            case TOKEN_LESS_THAN:
                return "LESS_THAN";
            case TOKEN_LESS_THAN_EQUAL:
                return "LESS_THAN_EQUAL";
            case TOKEN_GREATER_THAN:
                return "GREATER_THAN";
            case TOKEN_GREATER_THAN_EQUAL:
                return "GREATER_THAN_EQUAL";
            case TOKEN_EOS:
                return "EOS";
            case TOKEN_UNKNOWN:
                return "UNKNOWN";
            default:
                return "MISSING_STRING_EXPRESSION";
            }
        }

        long get_current_line_number() {
            return line_number_;
        }

        long get_current_cursor_point() {
            return 0;
        }

        template <typename Stream>
        void print_error_point(Stream& stream = std::cout) {
            // stream << current_line_ << std::endl;
            // for (int i = 0; i < get_current_cursor_point() - 1; ++i)
            //     stream << '_';
            // stream << '^';
        }
    };
}

#ifdef CURRENTIA_IS_LEXER_MAIN_
int main(int argc, char** argv)
{
    std::istringstream is("foo bar baz");
    currentia::Lexer lexer(&is);

    lexer.get_next_token();
    lexer.get_next_token();
    lexer.get_next_token();
    lexer.get_next_token();

    // std::istringstream is("");

    // currentia::Lexer lexer(&is);
    // currentia::Lexer::Token token;

    // while ((token = lexer.get_next_token()) != currentia::Lexer::TOKEN_EOS) {
    //     std::cout << "token => " << currentia::Lexer::token_to_string(token)
    //               << " '" << lexer.get_token_text() << "'" << std::endl;
    // }

    return 0;
}
#endif

#endif  /* ! CURRENTIA_QUERY_LEXER_H_  */
