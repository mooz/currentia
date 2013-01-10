#!/bin/sh

exec_experiment() {
    i=$1
    shift
    (cd $(git rev-parse --show-cdup); \
        ./build/src/currentia/server/experiment_scheduling --no-color --method=2pl --max-events-n-consume=${i} --update-interval=10000 $* < scheduling_query.cpl | tail)
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
