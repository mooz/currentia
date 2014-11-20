# Currentia

Currentia is a data stream management system (DSMS) that aims to
realize the concept of *data stream processing with concurrency
control*.

## Requirements

- C++ compiler that supports C++11 (`g++` or `clang++`)
- lemon (http://www.hwaci.com/sw/lemon/) for parser generation
- re2c (http://re2c.org/) for lexer generation
- gtest (http://code.google.com/p/googletest/) for testing

## Installation

### Ubuntu

Install `lemon` and `re2c` commands.

    sudo apt-get install lemon re2c

Then, make project.

    make configure && make

## References

- Masafumi Oyamada, Hideyuki Kawashima, and Hiroyuki Kitagawa, "Data Stream Processing with Concurrency Control", SIGAPP Appl. Comput. Rev., Vol. 13, No. 2, pp. 54-65, June 2013
  - URL: http://dl.acm.org/citation.cfm?id=2505425
  - PDF: http://www.sigapp.org/acr/Issues/V13.2/ACR-13-2-2013.pdf
- Masafumi Oyamada, Hideyuki Kawashima, and Hiroyuki Kitagawa, "Continuous Query Processing with Concurrency Control: Reading Updatable Resources Consistently", Proc. 28th ACM Symposium on Applied Computing (SAC 2013), Coimbra, Portugal, pp. 788-794, March 18-22, 2013.
  - URL: http://dl.acm.org/citation.cfm?id=2480362.2480514
