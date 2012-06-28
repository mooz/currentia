#!/bin/sh

(cd $(git rev-parse --show-cdup); waf generate_lexer_parser) && \
./tools/build_lexer.sh && \
./lexer.out
