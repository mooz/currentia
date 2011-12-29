// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PARSER_H_
#define CURRENTIA_QUERY_PARSER_H_

#include "currentia/query/lexer.h"

namespace currentia {
    class Parser {
        Lexer::ptr_t lexer_ptr_;

        Lexer::Token current_token_;
        Lexer::Token get_next_token_() {
            current_token_ = lexer_ptr_->next_token();
            return current_token_;
        }

        const std::string dummy_current_string_;
        const std::string& get_current_string_() {
            switch (current_token_) {
            case Lexer::NAME:
                return lexer_ptr_->get_latest_name();
            case Lexer::INTEGER:
            case Lexer::FLOAT:
                return lexer_ptr_->get_latest_number();
            case Lexer::STRING:
                return lexer_ptr_->get_latest_string();
            default:
                return dummy_current_string_;
            }
        }

    public:
        Parser(Lexer::ptr_t lexer_ptr):
            lexer_ptr_(lexer_ptr),
            current_token_(Lexer::UNKNOWN) {
        }

        void parse() {
            try {
                while (get_next_token_() != Lexer::EOF) {
                    std::cout << Lexer::token_to_string(current_token_);

                    switch (current_token_) {
                    case Lexer::NAME:
                    case Lexer::INTEGER:
                    case Lexer::FLOAT:
                    case Lexer::STRING:
                        std::cout << "(" << get_current_string_() << ")";
                        break;
                    default:
                        break;
                    }

                    std::cout << std::endl;
                }
            } catch (std::string error) {
                std::cerr << "\nSyntax Error: " << error << std::endl;
            }
        }
    };
}

#endif  /* ! CURRENTIA_QUERY_PARSER_H_  */
