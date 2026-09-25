project(
  'chestql',
  'c',
  default_options: {
    'default_library': 'static',
    'c_std': 'gnu99',
    'warning_level': '2',
  },
)

inc = include_directories('include')
src = []

subdir('src')

# dependencies ---

sqlite3 = dependency('sqlite3')
criterion = dependency('criterion')

deps = [sqlite3, criterion]

# ----------------

executable(
  meson.project_name(),
  sources: src,
  include_directories: inc,
  dependencies: deps,
)
