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

#include "currentia/core/operator/comparator.h"
#include "currentia/query/parser.h"
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

#define log(str) std::cout << str << std::endl
#define dump(x) std::cout << #x << " => [" << (x) << "]" << std::endl
#define TOKEN_EOS 0

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

        static std::string token_to_string(int token) {
            return "NULL";
            // switch (token) {
            //     CURRENTIA_DEFINE_TOKEN_LIST(CURRENTIA_DEFINE_SWITCH_STRING)
            // default:
            //     return "UNEXPECTED_TOKEN";
            // };
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

        int get_next_token() {
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

              'SELECT' 'ION'?   { return TOKEN_SELECT; }
              'PROJECT' 'ION'?  { return TOKEN_PROJECT; }

              'FROM'            { return TOKEN_FROM; }

              'AND'             { return TOKEN_AND; }
              'OR'              { return TOKEN_OR; }

              'SUM' 'MATION?'   { return TOKEN_SUM; }
              'MEAN'            { return TOKEN_MEAN; }
              'ELECT' 'ION'?    { return TOKEN_ELECT; }

              'STREAM'          { return TOKEN_STREAM; }
              'RELATION'        { return TOKEN_RELATION; }

              'NOT'             { return TOKEN_NOT; }

              'RECENT'          { return TOKEN_RECENT; }
              'ROWS'            { return TOKEN_ROWS; }
              'MSEC'            { return TOKEN_MSEC; }
              'SEC'             { return TOKEN_SEC; }
              'MIN'             { return TOKEN_MIN; }
              'HOUR'            { return TOKEN_HOUR; }
              'DAY'             { return TOKEN_DAY; }

              'INT'             { return TOKEN_TYPE_INT; }
              'FLOAT'           { return TOKEN_TYPE_FLOAT; }
              'STRING'          { return TOKEN_TYPE_STRING; }
              'BLOB'            { return TOKEN_TYPE_BLOB; }

              'SLIDE'           { return TOKEN_SLIDE; }

              IDENTIFIER        { return TOKEN_NAME; }

              STRING            { return TOKEN_STRING; }
              DIGIT+ [.] DIGIT* { return TOKEN_FLOAT; }
              DIGIT+            { return TOKEN_INTEGER; }

              ":"               { return TOKEN_COLON; }
              "{"               { return TOKEN_LBRACE; }
              "}"               { return TOKEN_RBRACE; }
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

        static bool is_token_conjunctive(int token) {
            return token == TOKEN_AND || token == TOKEN_OR;
        }

        static bool is_token_value(int token) {
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

        static currentia::Comparator::Type
        token_to_comparator(int comparator_type) {
            using namespace currentia::Comparator;

            switch (comparator_type) {
            case TOKEN_EQUAL:
                return EQUAL;
            case TOKEN_NOT_EQUAL:
                return NOT_EQUAL;
            case TOKEN_LESS_THAN:
                return LESS_THAN;
            case TOKEN_LESS_THAN_EQUAL:
                return LESS_THAN_EQUAL;
            case TOKEN_GREATER_THAN:
                return GREATER_THAN;
            case TOKEN_GREATER_THAN_EQUAL:
                return GREATER_THAN_EQUAL;
            default:
                throw "comparator type not given";
            };
        }

        static int parse_int(const std::string& str) {
            std::stringstream ss;
            int value_int;
            ss << str;
            ss >> value_int;
            return value_int;
        }

        static double parse_float(const std::string& str) {
            std::stringstream ss;
            double value_double;
            ss << str;
            ss >> value_double;
            return value_double;
        }
    };
}

#ifdef CURRENTIA_IS_LEXER_MAIN_

#include "parser.c"

int main(int argc, char** argv)
{
    std::istringstream is(
        " stream purchases(goods_id: int, user_id: int)"
        " relation goods(id: int, price: int)"
        " "
        " stream result"
        " from purchases, goods [purchases.id = goods.goods_id]"
        " {"
        "   selection goods.price < 5000"
        "   mean goods.price recent 5 rows slide 5 rows"
        " }"
    );
    currentia::Lexer lexer(&is);
    int token;

    currentia::yyParser* parser = reinterpret_cast<currentia::yyParser*>(currentia::CPLParseAlloc(malloc));
    currentia::CPLContainer cpl_container;
    std::string current_token_string;
    cpl_container.current_token_string = &current_token_string;

    while ((token = lexer.get_next_token()) != TOKEN_EOS) {
        // std::cout << "token => " << currentia::Lexer::token_to_string(token)
        //           << " '" << lexer.get_token_text() << "'" << std::endl;
        current_token_string = lexer.get_token_text();
        currentia::CPLParse(parser, token, new std::string(current_token_string), &cpl_container);
    }

    currentia::CPLParse(parser, TOKEN_EOS, NULL, &cpl_container);
    currentia::CPLParseFree(parser, free);

    return 0;
}
#endif

#endif  /* ! CURRENTIA_QUERY_LEXER_H_  */
