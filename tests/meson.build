# Tests registrados en tareas posteriores
test_lexer = executable('test_lexer', 'test_lexer.c',
  link_with: engine_lib,
  include_directories: engine_inc,
  dependencies: m_dep)
test('lexer', test_lexer)

test_eval = executable('test_eval', 'test_eval.c',
  link_with: engine_lib,
  include_directories: engine_inc,
  dependencies: m_dep)
test('eval', test_eval)

test_memory = executable('test_memory', 'test_memory.c',
  link_with: engine_lib,
  include_directories: engine_inc,
  dependencies: m_dep)
test('memory', test_memory)
