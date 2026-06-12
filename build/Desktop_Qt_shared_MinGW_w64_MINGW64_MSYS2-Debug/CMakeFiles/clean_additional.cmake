# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\prng_qt_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\prng_qt_autogen.dir\\ParseCache.txt"
  "prng_qt_autogen"
  )
endif()
