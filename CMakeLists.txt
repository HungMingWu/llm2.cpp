cmake_minimum_required(VERSION 3.28)
project(llm.cpp LANGUAGES CXX)

file(GLOB_RECURSE LLM_FORMAT_SOURCES CONFIGURE_DEPENDS
  LIST_DIRECTORIES false
  "${CMAKE_SOURCE_DIR}/*.h"
  "${CMAKE_SOURCE_DIR}/*.cpp"
  "${CMAKE_SOURCE_DIR}/*.cppm"
)
list(FILTER LLM_FORMAT_SOURCES EXCLUDE REGEX "^${CMAKE_BINARY_DIR}/")

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
  add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${LLM_FORMAT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting C++ sources with clang-format"
    VERBATIM
  )
  add_custom_target(format-check
    COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror ${LLM_FORMAT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking C++ formatting with clang-format"
    VERBATIM
  )
else()
  message(STATUS "clang-format not found; format and format-check targets are unavailable")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(OpenMP REQUIRED) 

add_subdirectory(base)
add_subdirectory(models)
add_subdirectory(cpu_backend)

add_executable(demo)
target_sources(demo
PRIVATE
  FILE_SET hidden_modules
  TYPE CXX_MODULES
  FILES
    cli.cppm
PRIVATE
  demo.cpp
)

set_target_properties(demo PROPERTIES
  OUTPUT_NAME run 
  CXX_EXTENSIONS OFF 
)

target_compile_options(demo PRIVATE
  -Wall -Wextra -freflection
  $<$<CONFIG:Release>:-O3>
  -march=native
)

target_link_options(demo PRIVATE
  -static-libstdc++
  -static-libgcc
)

target_link_libraries(demo PRIVATE
  libbase
  libcpu_backend
  libmodels
)
