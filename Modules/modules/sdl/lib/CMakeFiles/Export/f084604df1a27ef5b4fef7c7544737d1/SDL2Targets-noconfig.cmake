#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SDL2::SDL2" for configuration ""
set_property(TARGET SDL2::SDL2 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(SDL2::SDL2 PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libSDL2-2.0.0.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/libSDL2-2.0.0.dylib"
  )

list(APPEND _cmake_import_check_targets SDL2::SDL2 )
list(APPEND _cmake_import_check_files_for_SDL2::SDL2 "${_IMPORT_PREFIX}/lib/libSDL2-2.0.0.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
