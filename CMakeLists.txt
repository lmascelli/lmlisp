cmake_minimum_required(VERSION 3.12)
project(lmlisp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  runtime.hpp     runtime.cpp     
  types.hpp       types.cpp
  reader.hpp      reader.cpp
  printer.hpp     printer.cpp
  lmlisp.hpp      lmlisp.cpp
  core.hpp	  core.cpp)
list(TRANSFORM SOURCES PREPEND "src/")
add_library(lmlisp ${SOURCES})
set_target_properties(lmlisp PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
add_executable(test
  src/main_w.cpp)
target_link_libraries(test lmlisp readline)
elseif(UNIX)
add_executable(test
  src/main_l.cpp)
target_link_libraries(test lmlisp readline)
endif()
