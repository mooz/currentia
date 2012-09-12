#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Usage: $0 RESULT_DIRECTORY1 [ RESULT_DIRECTORY2 ... ]"
    exit
fi

plot()
{
    data=$1
    xlabel=$2
    ylabel=$3

    if [ -z $4 ]; then
        logscale=""
    else
        logscale="set logscale x"
    fi

    line_width=3
    point_size=1.5

    naive_label="Naive"
    lock_label="Lock"
    snapshot_label="Snapshot"

    (cat <<EOF
# set style line 1 lw ${line_width} lc -1 pt 2  ps ${point_size}     # Naive
# set style line 2 lw ${line_width} lc -1 pt 9 ps ${point_size}     # Lock
# set style line 3 lw ${line_width} lc -1 pt 5  ps ${point_size}     # Snapshot

set style line 2 lt 1 lc rgbcolor "#121212" lw ${line_width} pt 7 ps ${point_size}
set style line 3 lt 9 lc rgbcolor "#121212" lw ${line_width} pt 9 ps ${point_size}

${logscale}

set key outside center top horizontal reverse Left
# set key box
# set key below

set xlabel "${xlabel}"
set ylabel "${ylabel}"

set terminal svg
set term svg font "DejaVu Serif"

# "${data}" ind 0:0 usi 1:2 ti "${naive_label}" w lp linestyle 1,\

plot \
 "${data}" ind 1:1 usi 1:2 ti "${lock_label}" w lp linestyle 2,\
 "${data}" ind 2:2 usi 1:2 ti "${snapshot_label}" w lp linestyle 3

pause -1
EOF
    ) | gnuplot&
}

# i=1
# for directory in $*; do
#     BASEDIR=$(echo ${directory} | sed 's/\/$//')

#     ./show_query_vs_update.sh ${BASEDIR}  > /tmp/query_vs_update_${i}.txt
#     ./show_update_vs_stream.sh ${BASEDIR} > /tmp/update_vs_stream_${i}.txt
#     ./show_update_vs_window.sh ${BASEDIR} > /tmp/update_vs_window_${i}.txt

#     i=$(echo "$i + 1" | bc)
# done

# calculate average

BASEDIR=$(echo ${1} | sed 's/\/$//')

for i in $(seq 1 5); do
    DIR=${BASEDIR}_${i}

    ./show_query_vs_update.sh ${DIR}  > /tmp/query_vs_update_${i}.txt
    ./show_update_vs_stream.sh ${DIR} > /tmp/update_vs_stream_${i}.txt
    ./show_update_vs_window.sh ${DIR} > /tmp/update_vs_window_${i}.txt
done

./calculate_average.rb /tmp/query_vs_update_1.txt /tmp/query_vs_update_2.txt /tmp/query_vs_update_3.txt /tmp/query_vs_update_4.txt /tmp/query_vs_update_5.txt > /tmp/query_vs_update.txt
./calculate_average.rb /tmp/update_vs_stream_1.txt /tmp/update_vs_stream_2.txt /tmp/update_vs_stream_3.txt /tmp/update_vs_stream_4.txt /tmp/update_vs_stream_5.txt > /tmp/update_vs_stream.txt
./calculate_average.rb /tmp/update_vs_window_1.txt /tmp/update_vs_window_2.txt /tmp/update_vs_window_3.txt /tmp/update_vs_window_4.txt /tmp/update_vs_window_5.txt > /tmp/update_vs_window.txt

plot /tmp/query_vs_update.txt "Update Arrival Rate (query/sec)" "Query Throughput (query/sec)" LOGSCALE > ./query_vs_update.svg
plot /tmp/update_vs_stream.txt "Stream Arrival Rate (tuple/sec)" "Update Throughput (update/sec)" LOGSCALE > ./update_vs_stream.svg
plot /tmp/update_vs_window.txt "Window Size (tuple)" "Update Throughput (update/sec)" > ./update_vs_window.svg
