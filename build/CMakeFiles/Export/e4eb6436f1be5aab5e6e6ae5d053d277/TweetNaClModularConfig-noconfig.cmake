#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TweetNaClModular::tweetnacl" for configuration ""
set_property(TARGET TweetNaClModular::tweetnacl APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(TweetNaClModular::tweetnacl PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libtweetnacl.so.2.0.0"
  IMPORTED_SONAME_NOCONFIG "libtweetnacl.so.2"
  )

list(APPEND _cmake_import_check_targets TweetNaClModular::tweetnacl )
list(APPEND _cmake_import_check_files_for_TweetNaClModular::tweetnacl "${_IMPORT_PREFIX}/lib/libtweetnacl.so.2.0.0" )

# Import target "TweetNaClModular::tweetnacl_static" for configuration ""
set_property(TARGET TweetNaClModular::tweetnacl_static APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(TweetNaClModular::tweetnacl_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libtweetnacl.a"
  )

list(APPEND _cmake_import_check_targets TweetNaClModular::tweetnacl_static )
list(APPEND _cmake_import_check_files_for_TweetNaClModular::tweetnacl_static "${_IMPORT_PREFIX}/lib/libtweetnacl.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
