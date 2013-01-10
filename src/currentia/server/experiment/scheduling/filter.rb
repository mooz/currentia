#!/usr/bin/ruby

test_string = STDIN.read

# <<EOS
# Parse accept!!
# Elapsed: 4.9788 secs
# Update Rate: 100 qps
# Query Throughput: 200.852 tps
# Update Throughput: 9.84173 qps
# Scheduler Batch Process Count: 10 tuples
# Consistent Rate: 1
# Window: [ width 20 | stride 5 ] (TUPLE)
# # of Reset Tuples: 14136
# # of Operator Evaluation Count: 11612
# Method: 2pl
# EOS

test_string.grep(/Scheduler Batch Process Count: (\d+)/) { |match|
  print $1
}

print " "

test_string.grep(/Query Throughput: ([\d\.]+)/) { |match|
  print $1
}

print " "

test_string.grep(/Evaluation Count: (\d+)/) { |match|
  print $1
}
