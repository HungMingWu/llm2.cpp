cmake_minimum_required(VERSION 3.28)
project(llm.cpp LANGUAGES CXX)

set(LLM2_FORMAT_SOURCES
  demo.cpp
  base/base.cppm
  cpu_backend/cpu_backend.cpp
  cpu_backend/cpu_backend.cppm
  models/models.cppm
)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
  add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${LLM2_FORMAT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting C++ sources with clang-format"
    VERBATIM
  )
  add_custom_target(format-check
    COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror ${LLM2_FORMAT_SOURCES}
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
  demo.cpp
)

set_target_properties(demo PROPERTIES
  OUTPUT_NAME run 
  CXX_EXTENSIONS OFF 
)

target_compile_options(demo PRIVATE
  -Wall -Wextra
  $<$<CONFIG:Release>:-O3>
  -march=native
)

target_link_libraries(demo PRIVATE
  libcpu_backend
  libmodels
)
