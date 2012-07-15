#!/usr/bin/ruby

class ResultAnalyser
  def initialize(path, x_name, y_name)
    @dirpath  = path
    @x_name   = x_name.to_sym
    @y_name   = y_name.to_sym
    parse
  end

  def parse()
    begin
      Dir.open(@dirpath) { |dir|
        paths = dir.to_a.sort.find_all { |path| path.index(".txt") }

        naive, _non_naive = paths.partition { |path| path.index("none") == 0 }
        optimistic, _pessimistic  = _non_naive.partition { |path| path.index("optimistic") == 0 }
        lock, snapshot = _pessimistic.partition { |path| path.index("2pl") == 0 }

        @res_naive      = get_results(naive)
        @res_optimistic = get_results(optimistic)
        @res_lock       = get_results(lock)
        @res_snapshot   = get_results(snapshot)
      }
    rescue Exception => e
      STDERR.print "Failed to open #{@dirpath} :: #{e}\n"

      STDERR.print e.backtrace.join("\n")
    end
  end

  def parse_result(result)
    response = {}
    response[:method] = result.match(/Method: (.*)$/)[1]
    response[:tuples] = result.match(/Tuples: (.*)$/)[1]
    response[:elapsed] = result.match(/Elapsed: (.*) secs$/)[1]
    response[:stream_rate] = result.match(/Stream Rate: (.*) tps$/)[1]
    response[:update_rate] = result.match(/Update Rate: (.*) qps$/)[1]
    response[:query_throughput] = result.match(/Query Throughput: (.*) tps$/)[1]
    response[:update_throughput] = result.match(/Update Throughput: (.*) qps$/)[1]
    response[:window_size] = result.match(/width ([0-9]+)/)[1]
    response[:window_slide] = result.match(/\| stride ([0-9]+)/)[1]
    response[:consistent_rate] = result.match(/Consistent Rate: (.*)$/)[1]
    consistent_match = result.match(/Redo: ([0-9]+) times/)
    if consistent_match
      response[:redo_count] = consistent_match[1]
    end
    response
  end

  def get_result_and_parse(path)
    parse_result(IO.read(path))
  end

  def get_results(paths)
    paths.map { |path|
      get_result_and_parse(@dirpath + "/" + path)
    }.sort{ |a, b|
      a[@x_name].to_f <=> b[@x_name].to_f
    }
  end

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

  def print_result(res)
    puts res.map { |r|
      [r[@x_name], r[@y_name], abbreviate_number(r[@x_name]), abbreviate_number(r[@y_name])].join(" ")
    }.join("\n")
  end

  attr_accessor :res_naive, :res_optimistic, :res_lock, :res_snapshot
end

# http://t16web.lanl.gov/Kawano/gnuplot/datafile2.html

if ARGV.length < 3
  puts "Usage: #{__FILE__} DIRECTORY X Y [METHOD]"
  exit
end

analyser = ResultAnalyser.new(ARGV[0], ARGV[1], ARGV[2])

if ARGV[3]
  case ARGV[3]
    when "none"
    res = analyser.res_naive
    when "optimistic"
    res = analyser.res_optimistic
    when "2pl"
    res = analyser.res_lock
    when "snapshot"
    res = analyser.res_snapshot
  end

  analyser.print_result(res)
else
  # naive
  analyser.print_result(analyser.res_naive)
  print "\n\n"

  # optimistic
  analyser.print_result(analyser.res_optimistic)
  print "\n\n"

  # lock (2pl)
  analyser.print_result(analyser.res_lock)
  print "\n\n"

  # snapshot
  analyser.print_result(analyser.res_snapshot)
end
