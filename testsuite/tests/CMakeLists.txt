cmake_minimum_required(VERSION 3.1)

project(testsuite)

file(GLOB TEST_SOURCES RELATIVE "${PROJECT_SOURCE_DIR}" *.cpp)
# list(REMOVE_ITEM TEST_SOURCES "")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

foreach(sourcefile ${TEST_SOURCES})
  string(REPLACE ".cpp" "" name ${sourcefile})
  add_executable("test_${name}" ${sourcefile})
if (0 AND MSVC)
  add_dependencies("test_${name}" gip_STATIC)
  target_link_libraries("test_${name}" gip_STATIC)
  target_link_libraries("test_${name}" ${BASE_LIB_STATIC})
  target_link_libraries("test_${name}" Ws2_32 Netapi32 Winmm)
else ()
  add_dependencies("test_${name}" gip)
  target_compile_definitions("test_${name}" PRIVATE _COM_AZURE_DEV__GIP__SHARED_LIBRARY)
  target_compile_definitions("test_${name}" PRIVATE _COM_AZURE_DEV__BASE__SHARED_LIBRARY)
  target_link_libraries("test_${name}" gip ${BASE_LIB})
endif ()
  set_target_properties("test_${name}" PROPERTIES FOLDER testsuite)

  install(TARGETS "test_${name}"
    CONFIGURATIONS Debug
    ARCHIVE DESTINATION "${CMAKE_BINARY_DIR}/install/bin/debug"
    LIBRARY DESTINATION "${CMAKE_BINARY_DIR}/install/bin/debug"
    RUNTIME DESTINATION "${CMAKE_BINARY_DIR}/install/bin/debug"
  )

  install(TARGETS "test_${name}"
    CONFIGURATIONS Release
    ARCHIVE DESTINATION "${CMAKE_BINARY_DIR}/install/bin/release"
    LIBRARY DESTINATION "${CMAKE_BINARY_DIR}/install/bin/release"
    RUNTIME DESTINATION "${CMAKE_BINARY_DIR}/install/bin/release"
  )

endforeach()
