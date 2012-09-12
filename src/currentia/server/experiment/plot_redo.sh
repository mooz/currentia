#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Usage: $0 RESULT_DIRECTORY1 [ RESULT_DIRECTORY2 ... ]"
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

    line_width=5
    point_size=1.5

    optimistic_label="Optimistic"

    (cat <<EOF
set style line 1 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 5 ps ${point_size}
set style line 2 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 7 ps ${point_size}
set style line 3 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 9 ps ${point_size}
set style line 4 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 13 ps ${point_size}

${logscale}

set key outside center top horizontal reverse Left
# set key box
# set key below

set xlabel "${xlabel}"
set ylabel "${ylabel}"

set terminal svg
set term svg font "DejaVu Serif"

plot "${data}" ind 0:0 usi 1:2 ti "${optimistic_label}" w lp linestyle 2

pause -1
EOF
    ) | gnuplot&
}

./format_benchmark.rb ${BASEDIR}/query_vs_update update_rate redo_count optimistic > /tmp/update_vs_redo.txt
plot /tmp/update_vs_redo.txt "Update Arrival Rate (query/sec)" "Redo Count" LOGSCALE > ./update_vs_redo.svg
