#!/usr/bin/ruby
# -*- coding: utf-8 -*-

# ./add_border_eps.rb < test.eps > test_bordered.eps

def add_border_to_gnuplot_points(eps)
  eps.gsub!(/^\/([a-zA-Z]+F)\s*\{\s*stroke\s+\[\]\s+0\s+setdash\s+(.*?)\s*fill\s*\}\s*def/m) {
    point_name = $1
    point_shape = $2

    <<EOS
/#{point_name} {
  stroke [] 0 setdash

  % Draw point
  gsave
  2 copy
  #{point_shape}
  fill
  grestore

  % Draw border
  gsave
  0 0 0 setrgbcolor  % Fill with white color
  userlinewidth 2.5 div setlinewidth
  #{point_shape}
  stroke
  grestore
} def
EOS
  }
end

puts add_border_to_gnuplot_points(STDIN.read)

# test_string = <<EOS
# /TriUF {stroke [] 0 setdash vpt 1.12 mul add M
#   hpt neg vpt -1.62 mul V
#   hpt 2 mul 0 V
#   hpt neg vpt 1.62 mul V closepath fill} def
# EOS
