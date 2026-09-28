cmake_minimum_required(VERSION 3.28)
project(llm.cpp LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(OpenMP REQUIRED) 

add_subdirectory(models)

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
  OpenMP::OpenMP_CXX
  m
  libmodels
)
