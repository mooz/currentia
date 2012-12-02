#!/bin/sh

# if [ $# -lt 2 ]; then
#     echo "Usage: $0 DIRECTORY1 DIRECTORY2 ..."
# fi

for name in show_query_vs_update show_update_vs_consistency show_update_vs_window show_query_vs_window show_window_vs_consistency; do
    for i in $(seq 1 5); do
        DIR="result_${i}"
        ./${name}.sh ${DIR} > /tmp/${i}.txt
    done
    ./calculate_average.rb /tmp/1.txt /tmp/2.txt /tmp/3.txt /tmp/4.txt /tmp/5.txt > /tmp/average_${name}.txt
done
