#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Usage: $0 RESULT_DIRECTORY"
    exit
fi

BASEDIR=$(echo ${1} | sed 's/\/$//')

./format_benchmark.rb ${BASEDIR}/update_vs_window window_size query_throughput
