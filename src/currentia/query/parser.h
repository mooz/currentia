// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PARSER_H_
#define CURRENTIA_QUERY_PARSER_H_

#include "currentia/query/lexer.h"

namespace currentia {
    class Parser {
        Lexer::ptr_t lexer_ptr_;

    public:
        Parser(Lexer::ptr_t lexer_ptr):
            lexer_ptr_(lexer_ptr) {
        }

        void parse() {
            Lexer::Token token;
            try {
                while ((token = lexer_ptr_->next_token()) != Lexer::EOF) {
                    // std::cout << "Token => " << Lexer::token_to_string(token) << std::endl;
                    // if (token == Lexer::NAME)
                    //     std::cout << "Name => " << lexer_ptr_->get_latest_name() << std::endl;

                    std::cout << Lexer::token_to_string(token);

                    switch (token) {
                    case Lexer::NAME:
                        std::cout << "(" << lexer_ptr_->get_latest_name() << ")";
                        break;
                    case Lexer::INTEGER:
                    case Lexer::FLOAT:
                        std::cout << "(" << lexer_ptr_->get_latest_number() << ")";
                        break;
                    case Lexer::STRING:
                        std::cout << "(" << lexer_ptr_->get_latest_string() << ")";
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
