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

    line_width=5
    point_size=1.5

    naive_label="Naive"
    optimistic_label="Optimistic"
    lock_label="S2PL"
    snapshot_label="Snapshot"

# set terminal jpeg medium
# set output "histo.jpeg"
# set boxwidth 0.75 absolute
# set style fill solid 1.00 border -1
# set style data histogram
# set style histogram cluster gap 1
# set xtics 1000 nomirror
# set ytics 100 nomirror
# set mxtics 2
# set mytics 2
# set ytics 10
# set yrange [0:30]
# set ylabel "Total time"
# set xlabel "Session number"

# plot 'data' using 3 t "Server", '' using 4 t "Client", '' using 5:xtic(1) t "Network"

    (cat <<EOF
set style line 1 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 5 ps ${point_size}
set style line 2 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 7 ps ${point_size}
set style line 3 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 9 ps ${point_size}
set style line 4 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 13 ps ${point_size}

# ${logscale}

set key outside center top horizontal reverse Left
# set key box
# set key below

set style data histogram
set style histogram cluster gap 2
set style fill solid border -1
set boxwidth 0.9
# set xtic rotate by -45 scale 0 offset -2
set xtic scale 0 offset 0

set xlabel "${xlabel}"
set ylabel "${ylabel}"

set terminal svg
set term svg font "DejaVu Serif"

plot \
  "${data}" ind 0:0 usi 2:xticlabels(3) fs solid 1.0 lc rgbcolor "#000000" ti "${naive_label}",\
  "${data}" ind 1:1 usi 2:xticlabels(3) fs solid 1.0 lc rgbcolor "#585858" ti "${optimistic_label}",\
  "${data}" ind 2:2 usi 2:xticlabels(3) fs solid 1.0 lc rgbcolor "#b9b9b9" ti "${lock_label}",\
  "${data}" ind 3:3 usi 2:xticlabels(3) fs solid 1.0 lc rgbcolor "#FFFFFF" ti "${snapshot_label}"


#  "${data}" ind 0:0 usi 2:xticlabels(3) fs pattern 4 ti "${naive_label}",\
#  "${data}" ind 1:1 usi 2:xticlabels(3) fs pattern 3 ti "${optimistic_label}",\
#  "${data}" ind 2:2 usi 2:xticlabels(3) fs pattern 2 ti "${lock_label}",\
#  "${data}" ind 3:3 usi 2:xticlabels(3) fs pattern 6 ti "${snapshot_label}"


#  "${data}" ind 0:0 usi 2:xticlabels(3) fs solid 0.5 ti "${naive_label}",\
#  "${data}" ind 1:1 usi 2:xticlabels(3) fs solid 0.5 ti "${optimistic_label}",\
#  "${data}" ind 2:2 usi 2:xticlabels(3) fs solid 0.5 ti "${lock_label}",\
#  "${data}" ind 3:3 usi 2:xticlabels(3) fs solid 0.5 ti "${snapshot_label}"

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

BASEDIR=$(echo ${1} | sed 's/\/$//')

./format_benchmark.rb ${BASEDIR}/query_vs_update update_rate consistent_rate > /tmp/update_vs_consistency.txt
plot /tmp/update_vs_consistency.txt "Update Arrival Rate (query/sec)" "Consistent result ratio" LOGSCALE > ./update_vs_consistency.svg

./show_query_vs_update.sh ${BASEDIR}  > /tmp/query_vs_update.txt
plot /tmp/query_vs_update.txt "Update Arrival Rate (query/sec)" "Query Throughput (query/sec)" LOGSCALE > ./query_vs_update.svg

# ./show_update_vs_stream.sh ${DIR} > /tmp/update_vs_stream_${i}.txt
# plot /tmp/update_vs_stream.txt "Stream Arrival Rate (tuple/sec)" "Update Throughput (update/sec)" LOGSCALE > ./update_vs_stream.svg

# ./show_update_vs_window.sh ${DIR} > /tmp/update_vs_window_${i}.txt
# plot /tmp/update_vs_window.txt "Window Size (tuple)" "Update Throughput (update/sec)" > ./update_vs_window.svg
