#!/bin/sh

EXPERIMENT_BIN=$(git rev-parse --show-cdup)build/src/currentia/server/experiment_scheduling

exec_experiment() {
    i=$1
    shift
    cat scheduling_query.cpl | ${EXPERIMENT_BIN} --no-color --method=snapshot --max-events-n-consume=${i} --update-interval=10000 $*
}

for i in $(seq 1 10); do
    exec_experiment $i | ./filter.rb
    echo ""
done

echo "\n\n"

for i in $(seq 1 10); do
    exec_experiment $i --efficient-scheduling | ./filter.rb
    echo ""
done
