// -*- c++ -*-

#ifndef CURRENTIA_QUERY_CPL_PARSE_H_
#define CURRENTIA_QUERY_CPL_PARSE_H_

#include "currentia/query/cpl.h"
#include "currentia/query/cpl-parser.c"

namespace currentia {
    CPLQueryContainer::ptr_t parse_cpl(CPLLexer* lexer) {
        lemon::yyParser* yy_parser = reinterpret_cast<lemon::yyParser*>(lemon::CPLParseAlloc(malloc));
        CPLQueryContainer* query_container = new CPLQueryContainer();

        int token;
        while ((token = lexer->get_next_token()) != CPLLexer::TOKEN_EOS) {
            lemon::CPLParse(
                yy_parser,
                token,
                new std::string(lexer->get_token_text()),
                query_container
            );

            if (query_container->state == CPLQueryContainer::ERROR) {
                std::cerr << "Parse error at line " << lexer->get_current_line_number()
                          << " near token '" << lexer->get_token_text()
                          << "' (" << CPLLexer::token_to_string(token) << ")" << std::endl;
                break;
            }
        }

        if (query_container->state == CPLQueryContainer::NEUTRAL)
            lemon::CPLParse(yy_parser, CPLLexer::TOKEN_EOS, NULL, query_container);
        lemon::CPLParseFree(yy_parser, free);

        return CPLQueryContainer::ptr_t(query_container);
    }

    CPLQueryContainer::ptr_t parse_cpl(std::istream* ifs_ptr) {
        CPLLexer lexer(ifs_ptr);
        return parse_cpl(&lexer);
    }
};

#endif  /* ! CURRENTIA_QUERY_CPL_PARSE_H_ */
