APPNAME = 'libzbs'
VERSION = '0.1'

top = '.'
out = 'build'

def options(opt):
	opt.load('waf_unit_test')
	opt.load('compiler_cxx')
	# TODO: add assertion and bounds checks options

def configure(conf):
	conf.env.APPNAME = APPNAME
	conf.env.VERSION = VERSION
	conf.load('waf_unit_test')
	conf.load('compiler_cxx')
	conf.load('doxygen')
	conf.env.append_unique('CXXFLAGS', ['-std=c++11', '-Wall', '-Wextra', '-Werror'])
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
