cmake_minimum_required(VERSION 3.1)

project(testsuite)

file(GLOB TEST_SOURCES RELATIVE "${PROJECT_SOURCE_DIR}" *.cpp)
# list(REMOVE_ITEM TEST_SOURCES "")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

foreach(sourcefile ${TEST_SOURCES})
  string(REPLACE ".cpp" "" name ${sourcefile})
  add_executable("test_${name}" ${sourcefile})
if (NOT USE_SHARED)
  add_dependencies("test_${name}" gip_STATIC)
  target_link_libraries("test_${name}" gip_STATIC)
  target_link_libraries("test_${name}" base_STATIC)
else ()
  add_dependencies("test_${name}" gip)
  target_compile_definitions("test_${name}" PRIVATE _COM_AZURE_DEV__GIP__SHARED_LIBRARY)
  target_compile_definitions("test_${name}" PRIVATE _COM_AZURE_DEV__BASE__SHARED_LIBRARY)
  target_link_libraries("test_${name}" gip base)
endif ()
  set_target_properties("test_${name}" PROPERTIES FOLDER testsuite)

  install(TARGETS "test_${name}"
    CONFIGURATIONS Debug
    ARCHIVE DESTINATION "bin/debug"
    LIBRARY DESTINATION "bin/debug"
    RUNTIME DESTINATION "bin/debug"
  )

  install(TARGETS "test_${name}"
    CONFIGURATIONS Release
    ARCHIVE DESTINATION "bin/release"
    LIBRARY DESTINATION "bin/release"
    RUNTIME DESTINATION "bin/release"
  )

endforeach()

#if (WIN32)
#  set($ENV{PATH} "${CMAKE_SOURCE_DIR}/base/bin/${CMAKE_BUILD_TYPE};$ENV{PATH}")
#endif ()

enable_testing()

set (EXTENSION "")
set (CTEST_EXE "")
if (EMSCRIPTEN)
  set (EXTENSION ".js")
  set (CTEST_EXE node --experimental-wasm-threads --experimental-wasm-bulk-memory)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Wasm")
  set (CTEST_EXE wasmtime)
endif ()

if (NOT WIN32) # TAG: need base.dll in PATH
add_test(NAME test_gip COMMAND ${CTEST_EXE} ${CMAKE_CURRENT_BINARY_DIR}/test_gip${EXTENSION})
add_test(NAME test_types COMMAND ${CTEST_EXE} ${CMAKE_CURRENT_BINARY_DIR}/test_types${EXTENSION})
endif ()
