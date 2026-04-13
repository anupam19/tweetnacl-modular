# CMake generated Testfile for 
# Source directory: /workspace
# Build directory: /workspace/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(kat_selftest "/workspace/build/test_kat")
set_tests_properties(kat_selftest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;502;add_test;/workspace/CMakeLists.txt;0;")
add_test(FipsCavp "/workspace/build/test_cavp")
set_tests_properties(FipsCavp PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;514;add_test;/workspace/CMakeLists.txt;0;")
subdirs("tests")
