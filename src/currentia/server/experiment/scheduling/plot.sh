#!/bin/sh

if [ $# -lt 3 ]; then
    echo "Usage: $0 column x_label y_label"
    exit
fi

data_column=$1
xlabel=$2
ylabel=$3

cat > /tmp/input.txt
data_file=/tmp/input.txt

line_width=10
point_size=5

# Middle
color1="#347094"                # B
color2="#94346f"                # R
color3="#6f9434"                # G

FONT="Times New Roman"
BASE_FONT_SIZE=40
TICS_FONT_SIZE=30

data_label_1="Naive"
data_label_2="Constraint"

(cat <<EOF
# set logscale y

# ------------------------------------ #
# Line style
# ------------------------------------ #

set style line 1 lt 1 lc rgbcolor "${color1}" lw ${line_width} pt 9 ps ${point_size}
set style line 2 lt 1 lc rgbcolor "${color2}" lw ${line_width} pt 13 ps ${point_size}

# ------------------------------------ #
# Key (凡例) settings
# ------------------------------------ #

set key outside center horizontal top reverse Left
set key width 10
set key font "${FONT},${BASE_FONT_SIZE}"
# set key below

set decimal locale
set format y "%'g"

# ------------------------------------ #
# Tics (目盛)
# ------------------------------------ #

# offset x, y
set xtics offset 0, -1 font "${FONT},${TICS_FONT_SIZE}"
set ytics offset -1, 0 font "${FONT},${TICS_FONT_SIZE}"

# ------------------------------------ #
# Label
# ------------------------------------ #

set xlabel "${xlabel}" offset 0, -3 font "${FONT},${BASE_FONT_SIZE}"
set ylabel "${ylabel}" offset -5.5, 0 font "${FONT},${BASE_FONT_SIZE}T"
# [xoffset] [,yoffset]

# ------------------------------------ #
# Output format
# ------------------------------------ #

set terminal postscript eps

plot \
  "${data_file}" ind 0:0 usi ${data_column}:xticlabels(1) w lp linestyle 1 ti "${data_label_1}",\
  "${data_file}" ind 1:1 usi ${data_column}:xticlabels(1) w lp linestyle 2 ti "${data_label_2}"

pause -1
EOF
) | gnuplot | ./replace_filling.rb
