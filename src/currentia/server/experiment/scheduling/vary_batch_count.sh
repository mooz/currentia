#!/bin/sh

export BATCH_SIZE=1
export WIDTH=40
export SLIDE=20

for efficient in "" "--efficient-scheduling"; do
    for i in $(seq 1 10); do
        export BATCH_SIZE=${i}
        ./runner.sh ${efficient} | ./filter.rb
        echo ""
    done
    echo "\n\n"
done
