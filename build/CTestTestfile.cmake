# CMake generated Testfile for 
# Source directory: /workspace
# Build directory: /workspace/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(kat_selftest "/workspace/build/test_kat")
set_tests_properties(kat_selftest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;503;add_test;/workspace/CMakeLists.txt;0;")
subdirs("tests")
