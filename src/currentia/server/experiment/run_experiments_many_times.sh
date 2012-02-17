#!/bin/sh

for i in $(seq 1 5); do
    ./run_experiment.sh
    mv result result_${i}
done
