cmake_minimum_required(VERSION 3.28)

add_library(libmodels OBJECT)
target_sources(libmodels
PUBLIC
  FILE_SET private_interface
  TYPE CXX_MODULES
  FILES
    models.cppm
)

target_compile_features(libmodels PUBLIC cxx_std_26) 
