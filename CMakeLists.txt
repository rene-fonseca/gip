cmake_minimum_required(VERSION 3.1)

# see https://cmake.org/cmake-tutorial/

project(gip)

set(gip_VERSION_MAJOR 0)
set(gip_VERSION_MINOR 9)
set(gip_VERSION_MICRO 0)

#configure_file(
#  "${PROJECT_SOURCE_DIR}/configure.h.in"
#  "${PROJECT_BINARY_DIR}/configure.h"
#)

if (MSVC)
  add_definitions(-DUNICODE)
  add_definitions(-D_UNICODE)
endif ()

if (MSVC)
  # TAG: need to fix all exception specifications
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX") # no warnings allowed
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4290 /wd4838 /wd4244 /wd4267 /wd4996 /wd4297")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-exceptions")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror") # no warnings allowed
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated") # exception specifications
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overflow") # FIXME
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated-declarations") # FIXME
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror") # no warnings allowed
endif ()

endif ()

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/../base") # TAG: need depend on base lib

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
# TAG: add build date date --iso-8601=seconds

if (MSVC)
set(BUILD_DATE "Epoch") # TAG: FIXME
else ()
execute_process(OUTPUT_VARIABLE BUILD_DATE COMMAND date)
string(REGEX REPLACE "\n$" "" BUILD_DATE "${BUILD_DATE}")
endif ()
if (MSVC)
set(BUILD_DATE_SECONDS "0") # TAG: FIXME
else ()
execute_process(OUTPUT_VARIABLE BUILD_DATE_SECONDS COMMAND date "+%s")
string(REGEX REPLACE "\n$" "" BUILD_DATE_SECONDS "${BUILD_DATE_SECONDS}")
endif ()

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

include_directories(${CMAKE_BINARY_DIR}/generated)

add_library(gip SHARED ${gip_SRC} ${gip_ADDITIONAL} ${CMAKE_BINARY_DIR}/generated/gip/config.h)
target_compile_definitions(gip PUBLIC _DK_SDU_MIP__GIP__SHARED_LIBRARY)

if (APPLE)
# macOS requires all values to be less than 1024
set_target_properties(gip
  PROPERTIES
  VERSION ${gip_VERSION_MAJOR}.${gip_VERSION_MINOR}
  SOVERSION ${gip_VERSION_MAJOR})
else ()
set_target_properties(gip
  PROPERTIES
  VERSION ${gip_VERSION_MAJOR}.${gip_VERSION_MINOR}.${GIT_REVISION}
  SOVERSION ${gip_VERSION_MAJOR})
endif ()

add_library(gip_STATIC STATIC ${gip_SRC} ${CMAKE_BINARY_DIR}/generated/gip/config.h)
set_target_properties(gip_STATIC PROPERTIES OUTPUT_NAME gip_static)

find_library(BASE_LIB NAMES base libbase libbase.dylib PATHS base ../base ../base/build/Debug)
message(${BASE_LIB})

find_library(BASE_LIB_STATIC NAMES base_static libbase_static libbase_static.lib PATHS base ../base ../base/build/Debug)
message(${BASE_LIB_STATIC})

#add_dependencies(gip base)
target_link_libraries(gip ${BASE_LIB})

if (MSVC)
# target_link_libraries(gip Ws2_32 Netapi32 Winmm)
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

add_subdirectory("testsuite/tests")
