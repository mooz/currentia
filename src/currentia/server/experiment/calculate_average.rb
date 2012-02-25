#!/usr/bin/ruby

values = []

ARGV.each do |filename|
  File.open(filename) do |file|
    file.each.with_index do |line, nth|
      if line.length > 1
        x, y = line.split(" ").map {|str| str.to_f }
        if values[nth]
          # assert values[nth][0] == x
          values[nth][0] += x
          values[nth][1] += y
        else
          values[nth] = [x, y]
        end
      else
        # blank (gnuplot)
        values[nth] = nil
      end
    end
  end
end

puts values.map { |x, y|
  if x
    "#{x / ARGV.length} #{y / ARGV.length}"
  else
    ""
  end
}.join("\n")
