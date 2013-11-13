import os

cflags = '-Os -save-temps -fno-rtti -Wall -DGTEST_USE_OWN_TR1_TUPLE'
include_tests = ['./', 'vendor/gtest-1.7.0/include', 'vendor/gtest-1.7.0']

env = Environment(
		CC = 'clang',
		CXX = 'clang++',
		CCFLAGS = cflags,
		CXXFLAGS = cflags + ' -std=c++11 -stdlib=libc++',
		CPPPATH = include_tests,
		LINKFLAGS = '-stdlib=libc++',
		)
#without this stock path would get used, thus /usr/bin/$CXX would get invoked
env['ENV']['PATH'] = os.environ['PATH']

src_google_test = ['vendor/gtest-1.7.0/src/gtest-all.cc',
								'vendor/gtest-1.7.0/src/gtest_main.cc']
src = src_google_test + ['tests/' + i for i in ('TestBencoder.cpp', )]
headerness_src = ['tests/' + i for i in ('TestHeaderness1.cpp', 'TestHeaderness2.cpp')]

unit_tests = env.Program('unit_tests', src)
headerness = env.Program('headerness', headerness_src)

Default(unit_tests)
