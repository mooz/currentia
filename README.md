# Currentia

Currentia is a data stream management system (DSMS) that aims to
realize the concept of *transactional data stream processing*.

## Requirements

- C++ compiler that supports C++11 (`g++` or `clang++`)
- lemon (http://www.hwaci.com/sw/lemon/) for parser generation
- re2c (http://re2c.org/) for lexer generation
- gtest (http://code.google.com/p/googletest/) for testing

## Installation

### Ubuntu

Install  `lemon` and `re2c` commands.

    sudo apt-get install lemon re2c

Then, make project.

    make configure && make
