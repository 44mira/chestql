# testing dependencies ------------

criterion = dependency('criterion')

test_deps = [criterion]

#----------------------------------

test_parser_exec = executable(
  'test_parser',
  sources: files('test_parser.c', '../src/arena.c', '../src/parser.c'),
  include_directories: inc,
  dependencies: test_deps,
)

test('parser', test_parser_exec)
