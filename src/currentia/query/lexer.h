// -*- c++ -*-

#ifndef CURRENTIA_QUERY_LEXER_H_
#define CURRENTIA_QUERY_LEXER_H_

#include <list>
#include <iostream>
#include <algorithm>
#include <functional>
#include <ctype.h>

namespace currentia {
    class Lexer {
    public:
        typedef Lexer* ptr_t;

        enum Token {
            SELECT,                 // "SELECT"
            FROM,                   // "FROM"
            WHERE,                  // "WHERE"
            // conjunctives
            AND,                    // "AND"
            OR,                     // "OR"
            // names
            NAME,                   // [a-zA-Z_][a-zA-Z0-9_]*
            // others
            COMMA,                  // ","
            LPAREN,                 // "("
            RPAREN,                 // ")"
            // error
            UNKNOWN,
            EOF,
        };

        Lexer(std::istream* stream_ptr):
            stream_ptr_(stream_ptr) {
        }

        enum Token next_token() {
            while (is_char_available_()) {
                char next_char = peek_next_char_();
                // std::cout << "next char [" << next_char << "]" << std::endl;
                if (!is_char_available_())
                    return EOF;

                // TODO handle SELECTA as "NAME"
                switch (next_char) {
                case 'S':
                    if (peek_match_("SELECT"))
                        return rule_select_();
                    break;
                case 'F':
                    if (peek_match_("FROM"))
                        return rule_from_();
                    break;
                case 'W':
                    if (peek_match_("WHERE"))
                        return rule_where_();
                    break;
                case 'A':
                    if (peek_match_("AND"))
                        return rule_where_();
                    break;
                case 'O':
                    if (peek_match_("OR"))
                        return rule_or_();
                    break;
                case ',':
                    return rule_comma_();
                case '(':
                    return rule_lparen_();
                case ')':
                    return rule_rparen_();
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
                    return rule_name_();

                throw std::string("Unknown character ") + next_char;
            }

            return EOF;
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
            case NAME:
                return "NAME";
            case COMMA:
                return "COMMA";
            case LPAREN:
                return "LPAREN";
            case RPAREN:
                return "RPAREN";
            case EOF:
                return "EOF";
            case UNKNOWN:
            default:
                return "UNKNOWN";
            }
        }

        const std::string& get_latest_name() {
            return name_string_;
        }

    private:
        std::istream* stream_ptr_;

        typedef std::list<char> char_buffer_t;
        char_buffer_t next_char_buffer_;

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

        int peek_next_char_() {
            if (next_char_buffer_.empty())
                enqueue_next_char_buffer_(get_next_char_());
            return next_char_buffer_.front();
        }

        bool is_char_available_() {
            return !stream_ptr_->eof() && !next_char_buffer_.empty();
        }

        bool peek_match_(std::string expected) {
            bool matched = true;
            char_buffer_t consumed_chars;

            std::string::iterator iter = expected.begin();
            std::string::iterator iter_end = expected.end();
            for (; iter != iter_end; ++iter) {
                int next_char = get_next_char_();
                consumed_chars.push_back(next_char);
                if (*iter != next_char) {
                    matched = false;
                    break;
                }
            }

            std::for_each(consumed_chars.begin(),
                          consumed_chars.end(),
                          std::bind1st(std::mem_fun(&Lexer::enqueue_next_char_buffer_), this));

            return matched;
        }

        void match_(std::string expected) {
            std::string::iterator iter = expected.begin();
            std::string::iterator iter_end = expected.end();
            for (; iter != iter_end; ++iter) {
                if (*iter != get_next_char_())
                    throw std::string("Expected ") + expected;
            }
        }

        // rules
        std::string name_string_;
        enum Token rule_name_() {
            name_string_.clear();
            // [a-zA-Z][a-zA-Z0-9_]*
            if (is_alphabet(peek_next_char_()))
                name_string_.push_back(get_next_char_());
            while (is_identifier_component(peek_next_char_()))
                name_string_.push_back(get_next_char_());
            return NAME;
        }

        enum Token rule_select_() {
            match_("SELECT");
            return SELECT;
        }

        enum Token rule_from_() {
            match_("FROM");
            return FROM;
        }

        enum Token rule_where_() {
            match_("WHERE");
            return WHERE;
        }

        enum Token rule_and_() {
            match_("AND");
            return AND;
        }

        enum Token rule_or_() {
            match_("OR");
            return OR;
        }

        enum Token rule_comma_() {
            get_next_char_();
            return COMMA;
        }

        enum Token rule_lparen_() {
            get_next_char_();
            return COMMA;
        }

        enum Token rule_rparen_() {
            get_next_char_();
            return RPAREN;
        }

        void consume_comment_() {
            if (get_next_char_() != '#')
                throw std::string("Expected comment");

            while (is_char_available_() && peek_next_char_() != '\n')
                get_next_char_();
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_LEXER_H_  */
