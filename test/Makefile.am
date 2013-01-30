AM_CPPFLAGS = -I$(top_srcdir)/src
AM_LDFLAGS = $(top_builddir)/src/libzbs.la

check_PROGRAMS = \
	test_string \
	test_slice \
	test_vector

TESTS = $(check_PROGRAMS)

test_vector_SOURCES = test_vector.cc
test_string_SOURCES = test_string.cc
test_slice_SOURCES = test_slice.cc
