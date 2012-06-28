#!/usr/bin/env ruby

STDIN.lines.each { |line|
  matched = line.match(/#define ([a-zA-Z0-9_]+)/)
  puts "case #{matched[1]}: return \"#{matched[1]}\";" if matched
}
