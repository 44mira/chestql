project(
  'chestql',
  'c',
  default_options: {
    'default_library': 'static',
    'c_std': 'gnu99',
    'warning_level': '2',

    # disable subproject tests
    'criterion:tests': false,
    'criterion:samples': false,
    'boxfort:tests': false,
    'boxfort:samples': false,
  },
)

inc = include_directories('include')
src = []

subdir('src')

# dependencies ---

sqlite3 = dependency('sqlite3')

deps = [sqlite3]

# ----------------

executable(
  meson.project_name(),
  sources: src,
  include_directories: inc,
  dependencies: deps,
)

subdir('tests')
