project('mycalc', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11', 'warning_level=2'])

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: false)

engine_inc = include_directories('src')

engine_lib = static_library('calcengine',
  ['src/lexer.c', 'src/parser.c', 'src/evaluator.c',
   'src/memory_reg.c', 'src/history.c'],
  include_directories: engine_inc,
  dependencies: m_dep)

subdir('tests')

gtk_dep = dependency('gtk4')

executable('mycalc',
  ['src/main.c', 'src/calc_window.c'],
  include_directories: engine_inc,
  link_with: engine_lib,
  dependencies: [gtk_dep, m_dep],
  install: true)

install_data('data/io.github.sebastian.Calc.desktop',
  install_dir: get_option('datadir') / 'applications')
