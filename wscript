APPNAME = 'libzbs'
VERSION = '0.1'

top = '.'
out = 'build'

import sys

def options(opt):
	opt.load('waf_unit_test')
	opt.load('compiler_cxx')
	# TODO: add assertion and bounds checks options

def configure(conf):
	conf.env.APPNAME = APPNAME
	conf.env.VERSION = VERSION
	conf.load('waf_unit_test')

	if not conf.env['CXX'] and sys.platform == "darwin":
		conf.env['CXX'] = 'clang++'
	conf.load('compiler_cxx')
	conf.env.append_unique('CXXFLAGS', ['-std=c++11', '-Wall', '-Wextra'])
	if sys.platform == "darwin":
		# on darwin we force clang++ and libc++ at the moment as it's
		# the only option
		conf.env.CXXFLAGS_cxxshlib = ['-fPIC']
		conf.env.append_unique('CXXFLAGS', '-stdlib=libc++')
		conf.env.append_unique('LINKFLAGS', '-stdlib=libc++')

	conf.load('doxygen')
	# config headers?

	conf.define('ZBS_ENABLE_ASSERT', 1)
	conf.write_config_header('_config.h')

# use BuildContext for docs command (needs access to bld.env)
from waflib.Build import BuildContext
class DocsContext(BuildContext):
	cmd = 'docs'
	fun = 'docs'

def docs(bld):
	bld.recurse('docs')

def build(bld):
	bld.recurse('src test')
