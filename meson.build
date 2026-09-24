project(
  'chestql',
  'c',
  default_options: {'default_library': 'static', 'c_std': 'gnu99'},
)

inc = include_directories('include')
src = ['chestql.c']

# dependencies ---

dep_sqlite3 = dependency('sqlite3')

# ----------------

executable(
  meson.project_name(),
  sources: src,
  include_directories: inc,
  dependencies: [dep_sqlite3],
)
