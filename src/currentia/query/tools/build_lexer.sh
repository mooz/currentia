#!/bin/sh

g++ -pthread -g -O0 -I ../../ -DCURRENTIA_IS_LEXER_MAIN_ cpl-lexer.h -o lexer.out
