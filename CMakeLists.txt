cmake_minimum_required(VERSION 3.28)
project(gemma4 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(OpenMP REQUIRED) 

add_executable(gemma4 gemma4.cpp)
set_target_properties(gemma4 PROPERTIES
  OUTPUT_NAME run 
  CXX_EXTENSIONS OFF 
)

target_compile_features(gemma4 PRIVATE cxx_std_26) 

target_compile_options(gemma4 PRIVATE
  -Wall -Wextra
  $<$<CONFIG:Release>:-O3>
  -march=native
)

target_link_libraries(gemma4 PRIVATE
  OpenMP::OpenMP_CXX 
  m 
)
