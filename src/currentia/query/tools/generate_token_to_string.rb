#!/usr/bin/env ruby

STDIN.each_line { |line|
  matched = line.match(/#define ([a-zA-Z0-9_]+)/)
  puts "case #{matched[1]}: return \"#{matched[1]}\";" if matched
}
