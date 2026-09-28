cmake_minimum_required(VERSION 3.28)

add_library(libbase OBJECT)
target_sources(libbase
PUBLIC
  FILE_SET pmi
  TYPE CXX_MODULES
  FILES
    base.cppm
)

target_compile_features(libbase PUBLIC cxx_std_26) 
