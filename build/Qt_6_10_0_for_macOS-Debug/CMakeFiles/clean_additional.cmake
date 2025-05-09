# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "AvProjectUi_autogen"
  "CMakeFiles/AvProjectUi_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/AvProjectUi_autogen.dir/ParseCache.txt"
  )
endif()
