top = '.'
out = 'build'

def options(opt):
	opt.load('waf_unit_test')
	opt.load('compiler_cxx')
	# TODO: add assertion and bounds checks options

def configure(conf):
	conf.load('waf_unit_test')
	conf.load('compiler_cxx')
	conf.env.append_unique('CXXFLAGS', '-std=c++11')
	# config headers?

	conf.define('ZBS_ENABLE_ASSERT', 1)
	conf.write_config_header('_config.h')

def build(bld):
	bld.recurse('src test')
