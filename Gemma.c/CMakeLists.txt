cmake_minimum_required(VERSION 3.28)
project(Gemma LANGUAGES C CXX)

add_executable(Gemma)
target_sources(Gemma
PRIVATE
  gemma.cpp
  stb_image.c
)
target_compile_features(Gemma PUBLIC cxx_std_26)
target_link_libraries(Gemma PRIVATE
  libbase
)
target_link_options(Gemma PRIVATE
  -static-libstdc++
  -static-libgcc
)
