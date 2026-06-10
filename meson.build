project('mycalc', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11', 'warning_level=2'])

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: false)

engine_inc = include_directories('src')

engine_lib = static_library('calcengine',
  ['src/lexer.c', 'src/parser.c', 'src/evaluator.c', 'src/memory_reg.c'],
  include_directories: engine_inc,
  dependencies: m_dep)

subdir('tests')
