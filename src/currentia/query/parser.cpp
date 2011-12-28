// -*- c++ -*-

#include "currentia/query/parser.h"

int main(int argc, char **argv)
{
    using namespace currentia;

    Lexer  lexer(&std::cin);
    Parser parser(&lexer);
    parser.parse();

    return 0;
}
