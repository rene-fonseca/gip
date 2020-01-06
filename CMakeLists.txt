cmake_minimum_required(VERSION 3.1)

# see https://cmake.org/cmake-tutorial/

set(CMAKE_VERBOSE_MAKEFILE OFF)

project(gip)

if (COLOR)
  set (ENV{CLICOLOR_FORCE} 1)
endif ()

set(gip_VERSION_MAJOR 0)
set(gip_VERSION_MINOR 9)
set(gip_VERSION_MICRO 0)

# set(CMAKE_FIND_PACKAGE_PREFER_CONFIG 1)
find_package(base REQUIRED)
find_package(PNG)
find_package(JPEG)

if (MSVC)
  add_definitions(-DUNICODE)
  add_definitions(-D_UNICODE)
endif ()

if (CMAKE_CXX_COMPILER MATCHES "/wasic\\+\\+")
  set (CMAKE_CXX_COMPILER_ID "WASI")
endif ()

message("System: ${CMAKE_SYSTEM_NAME}")
message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_COM_AZURE_DEV__BASE__DEBUG")

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8") # force UTF-8 encoding for compiler
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # build in parallel
  if ("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # required for AMD64
  endif ()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX") # no warnings allowed
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4290 /wd4838 /wd4244 /wd4267 /wd4996 /wd4297")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") # TAG: DLL export issue  
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
  EMSCRIPTEN OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "WASI"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror") # no warnings allowed

  if (COLOR OR ("$ENV{CLICOLOR_FORCE}" STREQUAL "1"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics -fansi-escape-codes") # ansi colors
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated") # exception specifications
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror") # no warnings allowed

  if (COLOR OR ("$ENV{CLICOLOR_FORCE}" STREQUAL "1"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color") # ansi colors
  endif ()
endif ()

endif ()

file(GLOB_RECURSE gip_SRC
  "gip/*.h"
  "gip/*.cpp"
)

execute_process(OUTPUT_VARIABLE GIT_BRANCH COMMAND git symbolic-ref --short HEAD)
string(REGEX REPLACE "\n$" "" GIT_BRANCH "${GIT_BRANCH}")
execute_process(OUTPUT_VARIABLE GIT_COMMIT COMMAND git rev-parse HEAD)
string(REGEX REPLACE "\n$" "" GIT_COMMIT "${GIT_COMMIT}")
execute_process(OUTPUT_VARIABLE GIT_COMMIT_SHORT COMMAND git rev-parse --short HEAD)
string(REGEX REPLACE "\n$" "" GIT_COMMIT_SHORT "${GIT_COMMIT_SHORT}")
execute_process(OUTPUT_VARIABLE GIT_REVISION COMMAND git rev-list HEAD --count)
string(REGEX REPLACE "\n$" "" GIT_REVISION "${GIT_REVISION}")

string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)
string(TIMESTAMP BUILD_DATE_SECONDS "%s" UTC)

message("Branch: ${GIT_BRANCH}")
message("Commit: ${GIT_COMMIT}")
message("Revision: ${GIT_REVISION}")

# generate configuration header
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/generated/gip/config.h)

set(gip_ADDITIONAL "")
if (MSVC)
  configure_file(${CMAKE_SOURCE_DIR}/gip.rc.in ${CMAKE_BINARY_DIR}/generated/gip/gip.rc)
  set(gip_ADDITIONAL ${CMAKE_BINARY_DIR}/generated/gip/gip.rc)
endif ()

add_library(gip SHARED ${gip_SRC} ${gip_ADDITIONAL} ${CMAKE_BINARY_DIR}/generated/gip/config.h)
target_compile_definitions(gip PRIVATE _COM_AZURE_DEV__GIP__SHARED_LIBRARY_BUILD)
target_compile_definitions(gip PRIVATE _COM_AZURE_DEV__BASE__SHARED_LIBRARY) # TAG: define in header instead

target_include_directories(gip PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include> # <prefix>/include
)
target_include_directories(gip PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
  $<INSTALL_INTERFACE:include> # <prefix>/include
)

# macOS requires all values to be less than 1024
set_target_properties(gip
  PROPERTIES
  VERSION ${gip_VERSION_MAJOR}.${gip_VERSION_MINOR}
  SOVERSION ${gip_VERSION_MAJOR})

add_library(gip_STATIC STATIC ${gip_SRC} ${CMAKE_BINARY_DIR}/generated/gip/config.h)
set_target_properties(gip_STATIC PROPERTIES OUTPUT_NAME gip_static)

target_include_directories(gip_STATIC PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include> # <prefix>/include
)
target_include_directories(gip_STATIC PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
  $<INSTALL_INTERFACE:include> # <prefix>/include
)

target_link_libraries(gip PUBLIC base)
target_link_libraries(gip_STATIC PUBLIC base_STATIC)
if (PNG_FOUND)
target_link_libraries(gip PRIVATE PNG::PNG)
endif ()
if (JPEG_FOUND)
target_link_libraries(gip PRIVATE JPEG::JPEG)
endif ()

foreach(source IN ITEMS ${gip_SRC})
  get_filename_component(source_path "${source}" PATH)
  file(RELATIVE_PATH source_path_rel "${PROJECT_SOURCE_DIR}/gip" "${source_path}")
  string(REPLACE "/" "\\" group_path "${source_path_rel}")
  source_group("${group_path}" FILES "${source}")
  #message ("${group_path} -> ${source}")
endforeach()

# avoid no symbols warnings
if (APPLE)
  set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif ()

# preserve folder structure
install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/gip" # source directory
  DESTINATION "include" # target directory
  FILES_MATCHING PATTERN "*.h" # select header files
)
install(
  DIRECTORY "${CMAKE_BINARY_DIR}/generated/gip" # source directory
  DESTINATION "include" # target directory
  FILES_MATCHING PATTERN "*.h" # select header files
)

set_target_properties(gip PROPERTIES PUBLIC_HEADER "") # ${gip_HEADERS}

install(TARGETS gip gip_STATIC
  CONFIGURATIONS Debug
  EXPORT gipConfigDebug
  PUBLIC_HEADER DESTINATION "include/gip"
  ARCHIVE DESTINATION "lib/debug"
  LIBRARY DESTINATION "lib/debug"
  RUNTIME DESTINATION "bin/debug"
)

install(TARGETS gip gip_STATIC
  CONFIGURATIONS Release
  EXPORT gipConfigRelease
  PUBLIC_HEADER DESTINATION "include/gip"
  ARCHIVE DESTINATION "lib/release"
  LIBRARY DESTINATION "lib/release"
  RUNTIME DESTINATION "bin/release"
)

if (1)
export(TARGETS gip gip_STATIC
##  NAMESPACE gip::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/gipConfig.cmake"
)

install(EXPORT gipConfigDebug
  DESTINATION "lib/cmake/gip"
  CONFIGURATIONS Debug
##  NAMESPACE gip::
  FILE gipConfig.cmake
)
install(EXPORT gipConfigRelease
  DESTINATION "lib/cmake/gip"
  CONFIGURATIONS Release
##  NAMESPACE gip::
  FILE gipConfig.cmake
)
endif ()

add_subdirectory("testsuite/tests")

include(CTest)
enable_testing()

# TAG: add natvis

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ framework for developing image processing applications.")
SET(CPACK_PACKAGE_VENDOR "René Fonseca")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "${gip_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${gip_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
include(CPack)
