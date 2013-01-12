// -*- c++ -*-

#include "currentia/query/cpl.h"
#include "currentia/query/cpl-parser.c"

#include <fstream>
#include <string>
#include <iostream>

namespace currentia {
    void check_cpl(CPLLexer* lexer) {
        lemon::yyParser* yy_parser = reinterpret_cast<lemon::yyParser*>(lemon::CPLParseAlloc(malloc));
        CPLQueryContainer query_container;

        int token;
        while ((token = lexer->get_next_token()) != CPLLexer::TOKEN_EOS) {
            lemon::CPLParse(
                yy_parser,
                token,
                new std::string(lexer->get_token_text()),
                &query_container
            );

            if (query_container.state == CPLQueryContainer::ERROR) {
                std::stringstream ss;
                ss << lexer->get_current_line_number()
                   << ":1:"
                   << "Unexpected '" << CPLLexer::token_to_string(token)
                   << "' near token '" << lexer->get_token_text() << "'";
                throw ss.str();
            }
        }

        if (query_container.state == CPLQueryContainer::NEUTRAL)
            lemon::CPLParse(yy_parser, CPLLexer::TOKEN_EOS, NULL, &query_container);
        lemon::CPLParseFree(yy_parser, free);
    }

    void check_cpl(std::istream* ifs_ptr) {
        CPLLexer lexer(ifs_ptr);
        check_cpl(&lexer);
    }
};

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " query.cpl" << std::endl;
        exit(1);
    }

    const char* filename = argv[1];
    std::ifstream cpl_file((filename));

    try {
        currentia::check_cpl(&cpl_file);
    } catch (std::string error_message) {
        std::cerr << filename << ":" << error_message << std::endl;
    }

    return 0;
}
