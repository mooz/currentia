#!/usr/bin/ruby

values = []

def abbreviate_number(number)
  number = number.to_i
  base = 1000
  notations = ["", "K", "M", "G", "T", "P", "E", "Z", "Y"];
  while number >= base && notations.length > 1
    number = number /= base;
    notations.shift()
  end

  return [number.to_i, notations[0]].join("")
end

ARGV.each do |filename|
  File.open(filename) do |file|
    file.each.with_index do |line, nth|
      if line.length > 1
        x, y, _x_label, _y_label = line.split(" ").map {|s| s.to_f }

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
    x_avg = x / ARGV.length
    y_avg = y / ARGV.length
    "#{x_avg} #{y_avg} #{abbreviate_number x_avg} #{abbreviate_number y_avg}"
  else
    ""
  end
}.join("\n")
