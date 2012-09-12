#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Usage: $0 RESULT_DIRECTORY"
    exit
fi

BASEDIR=$(echo ${1} | sed 's/\/$//')

./format_benchmark.rb ${BASEDIR}/query_vs_update update_rate consistent_rate



