cmake_minimum_required(VERSION 3.30)
project(infoch VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

get_property(CMAKE_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_IS_MULTI_CONFIG)
  message(STATUS "not multi-config and not define build type, default: RelWithDebInfo")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(PROJECT_SOURCES src/main.cpp
                    src/base64.cpp
                    src/cli_utils.cpp
                    src/cli.cpp
                    src/terminal/tty.cpp
                    src/terminal/cbreak_mode.cpp
                    src/terminal/term_size.cpp
                    src/terminal/io.cpp
                    src/terminal/term_query.cpp
                    src/image/image.cpp
                    src/image/magick7.cpp
                    src/image/fn.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include/ src/)

find_package(PkgConfig REQUIRED)

find_package(fmt QUIET)
if(NOT fmt_FOUND)
  FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        12.1.0
  )
  FetchContent_MakeAvailable(fmt)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

option(USE_MAGICK "Magick as image backend (default)." ON)

find_package(ImageMagick 7.0.0 REQUIRED COMPONENTS Magick++)
target_link_libraries(${PROJECT_NAME} PRIVATE ImageMagick::Magick++)

option(USE_JEMALLOC "Use jemalloc." "AUTO")
if(USE_JEMALLOC STREQUAL "AUTO")
  pkg_check_modules(JEMALLOC QUIET jemalloc)

  if(JEMALLOC_FOUND)
    message(STATUS "Found jemalloc")

    target_include_directories(${PROJECT_NAME} PRIVATE ${JEMALLOC_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JEMALLOC_LIBRARIES})
  else()
    message(STATUS "Can not found jemalloc, use default from lib C/C++")
  endif()
elseif(USE_JEMALLOC STREQUAL "FORCE")
  message(STATUS "Enforce use jemalloc")
  pkg_check_modules(JEMALLOC REQUIRED jemalloc)

  target_include_directories(${PROJECT_NAME} PRIVATE ${JEMALLOC_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${JEMALLOC_LIBRARIES})
else()
  message(STATUS "Allocator: default on your lib C/C++")
endif()
