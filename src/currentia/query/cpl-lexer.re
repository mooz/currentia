// -*- c++ -*-
#ifndef CURRENTIA_QUERY_CPL_LEXER_H_
#define CURRENTIA_QUERY_CPL_LEXER_H_
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
#include <list>
#include <iostream>
#include <algorithm>
#include <functional>
#include <sstream>

#include <ctype.h>
#include <string.h>

#include "currentia/query/cpl-parser.h"
#include "currentia/trait/non-copyable.h"

namespace currentia {
    class CPLLexer : private NonCopyable<CPLLexer> {
    public:
        static const int TOKEN_EOS = 0;

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
        typedef CPLLexer* ptr_t;

        static std::string token_to_string(int token) {
            switch (token) {
                // cpl-token-to-string.h is generated by
                // $ tools/generate_token_to_string.rb < cpl-parser.h
#include "cpl-token-to-string.h"
            default:
                return "TOKEN_UNKNOWN";
            }
        }

        CPLLexer(std::istream* ifs_ptr, long buffer_init_size = 1024):
            ifs_ptr_(ifs_ptr),
            buffer_size_(buffer_init_size),
            line_number_(1) {
            // Allocate
            buffer_ = reinterpret_cast<char*>(malloc(sizeof(char) * buffer_size_));
            // Init
            yy_cursor_ = yy_marker_ = yy_limit_ = token_begin_ = buffer_;
        }

        ~CPLLexer() {
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

            if (remained_characters_count + n >= buffer_size_) {
                // Extend buffer
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
            memset(yy_limit_, '\0', sizeof(char) * empty_buffer_count);

            // Read characters from input stream
            ifs_ptr_->read(yy_limit_, empty_buffer_count);
            yy_limit_ += ifs_ptr_->gcount();
            // Dirty hack (for stringstream which doesn't give us a EOF character)
            if (ifs_ptr_->gcount() == 0 && *(yy_limit_ - 1) != '\0') {
                *(++yy_limit_) = '\0';
            }

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
              'COMBINE' | 'INJECT' { return TOKEN_COMBINE; }

              'FROM'            { return TOKEN_FROM; }

              'AND'             { return TOKEN_AND; }
              'OR'              { return TOKEN_OR; }

              'SUM' 'MATION?'   { return TOKEN_SUM; }
              'MEAN'            { return TOKEN_MEAN; }
              'ELECT' 'ION'?    { return TOKEN_ELECT; }

              'STREAM'          { return TOKEN_STREAM; }
              'RELATION'        { return TOKEN_RELATION; }

              'WHERE'           { return TOKEN_WHERE; }

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

#include "cpl-parser.c"

int main(int argc, char** argv)
{
    using namespace currentia;
    using namespace currentia::lemon;

    std::istringstream is(
        " stream purchases(goods_id: int, user_id: int)\n"
        " stream outliers(user_id: int)\n"
        " relation goods(id: int, price: int)\n"
        " \n"
        " stream result\n"
        " from purchases [recent 5 slide 5], outliers [recent 5 slide 5]\n"
        " where purchases.user_id = outliers.user_id\n"
        " {\n"
        "   combine goods where purchases.id = goods.goods_id\n"
        "   selection goods.price < 5000\n"
        "   mean goods.price [recent 5 slide 5]\n"
        " }\n"
    );
    CPLLexer lexer(&is);
    int token;

    yyParser* parser = reinterpret_cast<yyParser*>(CPLParseAlloc(malloc));
    CPLQueryContainer cpl_container;
    cpl_container.lexer = &lexer;

    try {
        while ((token = lexer.get_next_token()) != CPLLexer::TOKEN_EOS) {
            // int current_token;
            // std::cout << "token: " << lexer.get_token_text() << std::endl;
            CPLParse(parser,
                                token,
                                new std::string(lexer.get_token_text()),
                                &cpl_container);

            if (cpl_container.state == CPLQueryContainer::ERROR) {
                std::cerr << "Parse error at line " << lexer.get_current_line_number()
                          << " near token '" << lexer.get_token_text()
                          << "' (" << CPLLexer::token_to_string(token) << ")" << std::endl;
                break;
            }
        }

        if (cpl_container.state == CPLQueryContainer::NEUTRAL)
            CPLParse(parser, CPLLexer::TOKEN_EOS, NULL, &cpl_container);
        CPLParseFree(parser, free);
    } catch (const std::string& error_message) {
        std::cerr << "Fatal error: " << error_message << std::endl;
    }

    return 0;
}
#endif

#endif  /* ! CURRENTIA_QUERY_CPL_LEXER_H_  */
