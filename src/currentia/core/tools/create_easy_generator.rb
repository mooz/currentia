#!/usr/bin/env ruby

COUNT = 10

TYPENAMES = "%TYPENAMES%"
ARGS = "%ARGS%"
PUSH_BACKS = "%PUSH_BACKS%"

template = <<EOS
template <#{TYPENAMES}>
static
Tuple::ptr_t create_easy(const Schema::ptr_t& schema, #{ARGS}) {
    data_t data;
#{PUSH_BACKS}
    return Tuple::create(schema, data);
}
EOS

COUNT.times { |i|
  numbers = 1.upto(i + 1)

  typenames = numbers.map { |nth| "typename T#{nth}" }.join(", ")
  args = numbers.map { |nth| "const T#{nth}& t#{nth}" }.join(", ")
  push_backs = numbers.map { |nth| "    data.push_back(Object(t#{nth}));" }.join("\n")

  puts template.
  gsub(TYPENAMES, typenames).
  gsub(ARGS, args).
  gsub(PUSH_BACKS, push_backs)

  puts
}
