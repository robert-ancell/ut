project(
  'ut', 'c',
  version : '0.1',
  license : 'MPL-2.0',
  default_options: ['warning_level=1', 'c_std=c11'],
  meson_version : '>= 0.53.0'
)

add_project_arguments(['-D_POSIX_C_SOURCE=200809L'], language: 'c')

cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required: false)
thread_dep = dependency('threads')
rt_dep = cc.find_library('rt')

subdir('src')
