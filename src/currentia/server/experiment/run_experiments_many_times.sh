#!/bin/sh

for i in $(seq 1 10); do
    ./run_experiment.sh result_${i}
done
