cmake_minimum_required(VERSION 3.28)

add_library(libbase OBJECT)
target_sources(libbase
PRIVATE
  mmio.cpp
PUBLIC
  FILE_SET pmi
  TYPE CXX_MODULES
  FILES
    base.cppm
    mmio.cppm
)

if(WIN32)
  set(MMIO_PLATFORM_SOURCE mmio_win32.cppm)
elseif (UNIX)
  set(MMIO_PLATFORM_SOURCE mmio_linux.cppm)
else()
  message(FATAL_ERROR "mmio has no implementation for this platform")
endif()

target_sources(libbase
  PRIVATE
    FILE_SET os_dependent_modules
    TYPE CXX_MODULES
    FILES
      ${MMIO_PLATFORM_SOURCE}
)

target_compile_features(libbase PUBLIC cxx_std_26) 
