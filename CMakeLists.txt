cmake_minimum_required(VERSION 3.12)
project(lmlisp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  runtime.hpp     runtime.cpp     
  reader.hpp      reader.cpp
  lmlisp.hpp      lmlisp.cpp)
list(TRANSFORM SOURCES PREPEND "src/")
add_library(lmlisp ${SOURCES})
set_target_properties(lmlisp PROPERTIES LINKER_LANGUAGE CXX)

add_executable(test
  src/main.cpp)
target_link_libraries(test lmlisp)
