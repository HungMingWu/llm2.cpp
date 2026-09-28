cmake_minimum_required(VERSION 3.28)

find_package(OpenMP REQUIRED)

add_library(libcpu_backend OBJECT)
target_sources(libcpu_backend
PRIVATE
  cpu_backend.cpp
PUBLIC
  FILE_SET pmi
  TYPE CXX_MODULES
  FILES
    cpu_backend.cppm
)

target_compile_features(libcpu_backend PUBLIC cxx_std_26)

target_link_libraries(libcpu_backend
PUBLIC
  libbase
  OpenMP::OpenMP_CXX
  m
)

target_compile_options(libcpu_backend
PRIVATE
  -march=native
)

