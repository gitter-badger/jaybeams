# JayBeams uses a non-recursive make, this is the only Makefile.

instdir = $(prefix)/lib
inst_LIBRARIES = \
  jb/libjb.a

unit_tests = \
  jb/ut_as_hhmmss \
  jb/ut_config_files_location \
  jb/ut_config_object \
  jb/ut_logging \
  jb/ut_severity_level \
  jb/ut_strtonum

examples = \
  examples/configuration

noinst_PROGRAMS = $(examples)

check_PROGRAMS = $(unit_tests)
TESTS = $(check_PROGRAMS)

AM_CXXFLAGS = \
  -DJB_SYSCONFDIR=\"$(sysconfdir)\" \
  -DJB_BINDIR=\"$(bindir)\" \
  $(YAML_CPP_CFLAGS) \
  $(BOOST_CPPFLAGS) \
  -DBOOST_LOG_DYN_LINK \
  $(PTHREAD_CFLAGS)

LDFLAGS = \
  $(BOOST_LDFLAGS)

YAML_CPP_LIBS=-lyaml-cpp

LIBS = \
  $(YAML_CPP_LIBS) \
  $(BOOST_LOG_SETUP_LIB) \
  $(BOOST_LOG_LIB) \
  $(BOOST_FILESYSTEM_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_THREAD_LIB) \
  $(BOOST_PROGRAM_OPTIONS_LIB) \
  $(PTHREAD_LIBS)

# Common configuration for all unit tests
UT_CPPFLAGS = \
  -DBOOST_TEST_DYN_LINK \
  -DBOOST_TEST_MAIN \
  -DSKYE_USE_BOOST_UNIT_TEST_FRAMEWORK

jb_ut_libs = \
  jb/libjb.a \
  $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

@CODE_COVERAGE_RULES@
AM_CXXFLAGS += $(CODE_COVERAGE_CFLAGS)
LIBS += $(CODE_COVERAGE_LDFLAGS)

################################################################
# jb/libjb.a
################################################################

jb_libjb_adir = $(includedir)
nobase_jb_libjb_a_HEADERS = \
  jb/as_hhmmss.hpp \
  jb/config_attribute.hpp \
  jb/config_files_location.hpp \
  jb/config_object.hpp \
  jb/config_recurse.hpp \
  jb/convert_severity_level.hpp \
  jb/log.hpp \
  jb/severity_level.hpp \
  jb/strtonum.hpp \
  jb/usage.hpp
jb_libjb_a_SOURCES = \
  jb/as_hhmmss.cpp \
  jb/config_files_location.cpp \
  jb/config_object.cpp \
  jb/log.cpp \
  jb/severity_level.cpp

jb_libjb_a_LIBADD = 

################################################################
# examples
################################################################

examples_configuration_SOURCES = \
  examples/configuration.cpp
examples_configuration_LDADD = jb/libjb.a

################################################################
# unit tests and benchmarks
################################################################

jb_ut_as_hhmmss_SOURCES = \
  jb/ut_as_hhmmss.cpp
jb_ut_as_hhmmss_CPPFLAGS = \
  $(UT_CPPFLAGS) \
  -DBOOST_TEST_MODULE=jb_ut_as_hhmmss
jb_ut_as_hhmmss_LDADD = \
  $(jb_ut_libs)

jb_ut_config_files_location_SOURCES = \
  jb/ut_config_files_location.cpp
jb_ut_config_files_location_CPPFLAGS = \
  $(UT_CPPFLAGS) \
  -DBOOST_TEST_MODULE=jb_ut_config_files_location
jb_ut_config_files_location_LDADD = \
  $(jb_ut_libs)

jb_ut_config_object_SOURCES = \
  jb/ut_config_object.cpp
jb_ut_config_object_CPPFLAGS = \
  $(UT_CPPFLAGS) \
  -DBOOST_TEST_MODULE=jb_ut_config_object
jb_ut_config_object_LDADD = \
  $(jb_ut_libs)

jb_ut_logging_SOURCES = \
  jb/ut_logging.cpp
jb_ut_logging_CPPFLAGS = \
  $(UT_CPPFLAGS) \
  -DBOOST_TEST_MODULE=jb_ut_logging
jb_ut_logging_LDADD = \
  $(jb_ut_libs)

jb_ut_severity_level_SOURCES = \
  jb/ut_severity_level.cpp
jb_ut_severity_level_CPPFLAGS = \
  $(UT_CPPFLAGS) \
  -DBOOST_TEST_MODULE=jb_ut_severity_level
jb_ut_severity_level_LDADD = \
  $(jb_ut_libs)

jb_ut_strtonum_SOURCES = \
  jb/ut_strtonum.cpp
jb_ut_strtonum_CPPFLAGS = \
  $(UT_CPPFLAGS) \
  -DBOOST_TEST_MODULE=jb_ut_strtonum
jb_ut_strtonum_LDADD = \
  $(jb_ut_libs)