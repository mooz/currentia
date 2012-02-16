#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Usage: $0 RESULT_DIRECTORY"
    exit
fi

BASEDIR=$(echo ${1} | sed 's/\/$//')

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

    line_width=2
    point_size=1.0

    naive_label="Naive"
    lock_label="Lock"
    snapshot_label="Snapshot"

    (cat <<EOF
# set style line 1 lw ${line_width} lc -1 pt 2  ps ${point_size}     # Naive
set style line 2 lw ${line_width} lc -1 pt 9 ps ${point_size}     # Lock
set style line 3 lw ${line_width} lc -1 pt 5  ps ${point_size}     # Snapshot

${logscale}

set key box
set key below

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

./show_query_vs_update.sh ${BASEDIR} > /tmp/query_vs_update.txt
plot /tmp/query_vs_update.txt "Update Arrival Rate (query/sec)" "Query Throughput (query/sec)" LOGSCALE > ./query_vs_update.svg

./show_update_vs_stream.sh ${BASEDIR} > /tmp/update_vs_stream.txt
plot /tmp/update_vs_stream.txt "Stream Arrival Rate (tuple/sec)" "Update Throughput (update/sec)" LOGSCALE > ./update_vs_stream.svg

./show_update_vs_window.sh ${BASEDIR} > /tmp/update_vs_window.txt
plot /tmp/update_vs_window.txt "Window Size (tuple)" "Update Throughput (update/sec)" > ./update_vs_window.svg
