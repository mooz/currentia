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

line_width=1.7
point_size=1

naive_label="Naive"
optimistic_label="Optimistic"
lock_label="S2PL"
snapshot_label="Snapshot"

# Dim
naive_color="#202020"
optimistic_color="#174479"
lock_color="#781744"
snapshot_color="#447817"

# Pop
naive_color="#343434"
optimistic_color="#c23ea1"
lock_color="#3ea1c2"
snapshot_color="#a0c13e"

# Middle
naive_color="#343434"
optimistic_color="#a6368a"
lock_color="#8aa637"
snapshot_color="#378aa6"

#9ac225

(cat <<EOF
# ${logscale}

# ------------------------------------ #
# Line style
# ------------------------------------ #

set style line 1 lt 1 lc rgbcolor "${naive_color}"      lw ${line_width} pt 5 ps ${point_size} # Naive
set style line 2 lt 1 lc rgbcolor "${optimistic_color}" lw ${line_width} pt 7 ps ${point_size} # Optimistic
set style line 3 lt 1 lc rgbcolor "${lock_color}"       lw ${line_width} pt 9 ps ${point_size} # S2PL
set style line 4 lt 1 lc rgbcolor "${snapshot_color}"   lw ${line_width} pt 13 ps ${point_size} # Snapshot

# ------------------------------------ #
# Key (凡例) settings
# ------------------------------------ #

set key outside center horizontal top reverse Left
set key font "Times New Roman,26"
# set key box
# set key below

# ------------------------------------ #
# Tics (目盛)
# ------------------------------------ #

set tics font "Times New Roman,15"

# ------------------------------------ #
# Label
# ------------------------------------ #

set xlabel "${xlabel}" offset 0, 0 font "Times New Roman,28"
set ylabel "${ylabel}" offset 2 font "Times New Roman,28T"
# [xoffset] [,yoffset]

# ------------------------------------ #
# Output format
# ------------------------------------ #

set terminal svg
# set term svg font "DejaVu Serif,16"
set term svg font "Times New Roman,18"

plot \
  "${data_file}" ind 0:0 usi 2:xticlabels(3) w lp linestyle 1 ti "${naive_label}",\
  "${data_file}" ind 1:1 usi 2:xticlabels(3) w lp linestyle 2 ti "${optimistic_label}",\
  "${data_file}" ind 2:2 usi 2:xticlabels(3) w lp linestyle 3 ti "${lock_label}",\
  "${data_file}" ind 3:3 usi 2:xticlabels(3) w lp linestyle 4 ti "${snapshot_label}"

pause -1
EOF
) | gnuplot
