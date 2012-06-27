#!/bin/sh

re2c -i -o lexer.cc lexer.re && g++ -pthread -g -O0 -I ../../ -DCURRENTIA_IS_LEXER_MAIN_ lexer.cc -o lexer.out
