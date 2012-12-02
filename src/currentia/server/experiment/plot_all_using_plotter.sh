#!/bin/sh

# # Window
# ./show_update_vs_window.sh result/ | ./plotter_chart.sh "Window Range (tuples)" "Update Throughput (txn/s)" > window_range_update_throughput.svg
# ./show_query_vs_window.sh result/ | ./plotter_chart.sh "Window Range (tuples)" "Query Throughput (event/s)" > window_range_query_throughput.svg
# # Update
# ./show_query_vs_update.sh result/ | ./plotter_chart.sh "Update Arrival Rate (txn/s)" "Query Throughput (event/s)" > query_vs_update.svg
# # Consistent rate
# ./show_update_vs_consistency.sh result/ | ./plotter_chart.sh "Update Arrival Rate (txn/s)" "Consistent results ratio" > update_vs_consistency.svg

# ------------------------------------------------------------------- #

# Window
cat /tmp/average_show_update_vs_window.txt      | ./plotter_chart.sh "window range (tuples)" "update throughput (txn/s)" > window_range_update_throughput.svg
cat /tmp/average_show_query_vs_window.txt       | ./plotter_chart.sh "window range (tuples)" "CQ throughput (event/s)" > window_range_query_throughput.svg
cat /tmp/average_show_window_vs_consistency.txt | ./plotter_chart.sh "window range (tuples)" "proportion of consistent results" > window_vs_consistency.svg
# Update
cat /tmp/average_show_query_vs_update.txt       | ./plotter_chart.sh "update arrival rate (txn/s)" "CQ throughput (event/s)" > query_vs_update.svg
# Consistent rate
cat /tmp/average_show_update_vs_consistency.txt | ./plotter_chart.sh "update arrival rate (txn/s)" "proportion of consistent results" > update_vs_consistency.svg

