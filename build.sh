#!/bin/sh

CC="ccache clang" CXX="ccache clang++" waf configure && waf build
