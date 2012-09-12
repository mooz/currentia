#!/bin/sh

if [ $# -lt 2 ]; then
    echo "Usage: $0 XLABEL YLABEL"
    exit
fi

xlabel=$1
ylabel=$2

cat > /tmp/input.txt
data_file=/tmp/input.txt

# if [ -z $3 ]; then
#     logscale=""
# else
#     logscale="set logscale x"
# fi

line_width=5
point_size=1.5

naive_label="Naive"
optimistic_label="Optimistic"
lock_label="S2PL"
snapshot_label="Snapshot"

(cat <<EOF
# ${logscale}

# ------------------------------------ #
# Line style
# ------------------------------------ #

# set style line 1 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 5 ps ${point_size}
# set style line 2 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 7 ps ${point_size}
# set style line 3 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 9 ps ${point_size}
# set style line 4 lt 1 lc rgbcolor "#131313" lw ${line_width} pt 13 ps ${point_size}

# ------------------------------------ #
# Histogram Settings
# ------------------------------------ #

set style data histogram
set style histogram cluster gap 2
set style fill solid border -1
set boxwidth 0.9
set xtic scale 0 offset 0

# ------------------------------------ #
# Key (凡例) settings
# ------------------------------------ #

# set key outside center top reverse Left
# set key box
# set key below

# ------------------------------------ #
# Label
# ------------------------------------ #

# x label を回転させるかどうか
# set xtic rotate by -45 scale 0 offset -2

set xlabel "${xlabel}"
set ylabel "${ylabel}"

# ------------------------------------ #
# Output format
# ------------------------------------ #

set terminal svg
set term svg font "DejaVu Serif"

plot \
  "${data_file}" ind 0:0 usi 2:xticlabels(3) fs solid 1.0 lc rgbcolor "#000000" ti "${naive_label}",\
  "${data_file}" ind 1:1 usi 2:xticlabels(3) fs solid 1.0 lc rgbcolor "#585858" ti "${optimistic_label}",\
  "${data_file}" ind 2:2 usi 2:xticlabels(3) fs solid 1.0 lc rgbcolor "#b9b9b9" ti "${lock_label}",\
  "${data_file}" ind 3:3 usi 2:xticlabels(3) fs solid 1.0 lc rgbcolor "#FFFFFF" ti "${snapshot_label}"

pause -1
EOF
) | gnuplot&
