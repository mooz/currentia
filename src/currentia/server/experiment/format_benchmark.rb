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
        lock, versioning  = _non_naive.partition { |path| path.index("lock") == 0 }

        @res_naive      = get_results(naive)
        @res_lock       = get_results(lock)
        @res_versioning = get_results(versioning)
      }
    rescue Exception => e
      STDERR.print "Failed to open #{@dirpath} :: #{e}\n"

      STDERR.print e.backtrace.join("\n")
    end
  end

  def parse_result(result)
    {
      :method => result.match(/Method: (.*)$/)[1],
      :tuples => result.match(/Tuples: (.*)$/)[1],
      :elapsed => result.match(/Elapsed: (.*) secs$/)[1],
      :stream_rate => result.match(/Stream Rate: (.*) tps$/)[1],
      :update_rate => result.match(/Update Rate: (.*) qps$/)[1],
      :query_throughput => result.match(/Query Throughput: (.*) tps$/)[1],
      :update_throughput => result.match(/Update Throughput: (.*) qps$/)[1],
      :selectivity => result.match(/Selectivity: (.*)$/)[1],
      :window_size => result.match(/Window: (.*)$/)[1]
    }
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

  def print_result(res)
    puts res.map { |r|
      [r[@x_name], r[@y_name]].join(" ")
    }.join("\n")
  end

  attr_accessor :res_naive, :res_lock, :res_versioning
end

# http://t16web.lanl.gov/Kawano/gnuplot/datafile2.html

if ARGV.length < 3
  puts "Usage: #{__FILE__} DIRECTORY X Y"
  exit
end

analyser = ResultAnalyser.new(ARGV[0], ARGV[1], ARGV[2])

# naive
analyser.print_result(analyser.res_naive)
print "\n\n"

# lock
analyser.print_result(analyser.res_lock)
print "\n\n"

# versioning
analyser.print_result(analyser.res_versioning)
