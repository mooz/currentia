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
#include "currentia/query/token.h"

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

#define log(str) std::cout << str << std::endl
#define dump(x) std::cout << #x << " => [" << (x) << "]" << std::endl

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
            CURRENTIA_DEFINE_TOKEN_LIST(CURRENTIA_DEFINE_ENUM)
        };

        static std::string token_to_string(enum Token token) {
            switch (token) {
                CURRENTIA_DEFINE_TOKEN_LIST(CURRENTIA_DEFINE_SWITCH_STRING)
            default:
                return "UNEXPECTED_TOKEN";
            };
        }

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
            if (ifs_ptr_->eof() && (yy_cursor_ >= yy_limit_))
                return false;   // EOS

            int remained_characters_count = yy_limit_ - yy_cursor_;
            int yy_cursor_offset = yy_cursor_ - buffer_;
            int yy_marker_offset = yy_marker_ - buffer_;
            int yy_limit_offset = yy_limit_ - buffer_;

            // dump(yy_cursor_offset);
            // dump(yy_limit_offset);

            if (remained_characters_count + n >= buffer_size_) {
                // Extend buffer
                log("We need to extend buffer");
                buffer_ = reinterpret_cast<char*>(realloc(buffer_, buffer_size_ *= 2));
                // Adjust position
                yy_cursor_ = token_begin_ = buffer_;
                yy_marker_ = buffer_ + yy_marker_offset;
                yy_limit_  = buffer_ + yy_limit_offset;
            } else if (remained_characters_count > 0) {
                // Shift remained characters in the buffer_ to head of the buffer_
                memmove(buffer_, yy_cursor_, sizeof(char) * remained_characters_count);
                // Adjust position (shift)
                yy_cursor_ = token_begin_ = buffer_;
                yy_marker_ = buffer_ + (yy_marker_offset - yy_cursor_offset);
                yy_limit_  = buffer_ + (yy_limit_offset - yy_cursor_offset);
            }

            // This line is very important (fill buffer with EOF)
            int empty_buffer_count = (buffer_ + buffer_size_) - yy_limit_;
            // dump(empty_buffer_count);
            // dump(buffer_);
            memset(yy_limit_, '\0', sizeof(char) * empty_buffer_count);
            // dump(buffer_);

            // Read characters from input stream
            ifs_ptr_->read(yy_limit_, empty_buffer_count);
            yy_limit_ += ifs_ptr_->gcount();
            // Dirty hack (for stringstream which doesn't give us a EOF character)
            if (ifs_ptr_->gcount() == 0 && *(yy_limit_ - 1) != '\0') {
                *(++yy_limit_) = '\0';
            }

            // log("Read");
            // dump(buffer_);

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
              DIGIT+            { return TOKEN_INTEGER; }

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

#undef CURRENTIA_DEFINE_ENUM
#undef CURRENTIA_DEFINE_SWITCH_STRING
#undef CURRENTIA_DEFINE_TOKEN_LIST
}

#ifdef CURRENTIA_IS_LEXER_MAIN_
int main(int argc, char** argv)
{
    std::istringstream is("abc def ghi");
    currentia::Lexer lexer(&is);
    currentia::Lexer::Token token;

    while ((token = lexer.get_next_token()) != currentia::Lexer::TOKEN_EOS) {
        std::cout << "token => " << currentia::Lexer::token_to_string(token)
                  << " '" << lexer.get_token_text() << "'" << std::endl;
    }

    return 0;
}
#endif

#endif  /* ! CURRENTIA_QUERY_LEXER_H_  */
