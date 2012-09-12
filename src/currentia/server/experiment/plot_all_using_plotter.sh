#!/bin/sh

# Window
./show_update_vs_window.sh result/ | ./plotter_chart.sh "Window Range (tuples)" "Update Throughput (txn/s)" > window_range_update_throughput.svg
./show_query_vs_window.sh result/ | ./plotter_chart.sh "Window Range (tuples)" "Query Throughput (event/s)" > window_range_query_throughput.svg

# Update
./show_query_vs_update.sh result/ | ./plotter_chart.sh "Update Arrival Rate (txn/s)" "Query Throughput (event/s)" > query_vs_update.svg

# Consistent rate
# Table is enough?
./show_update_vs_consistency.sh result/ | ./plotter_chart.sh "Update Arrival Rate (txn/s)" "Consistent results ratio" > update_vs_consistency.svg
