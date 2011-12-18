#!/bin/sh

export CC="ccahe clang"
export CXX="ccache clang++"
waf configure && waf build
