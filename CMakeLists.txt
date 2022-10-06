cmake_minimum_required(VERSION 3.12)
project(mylisp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  lmlisp.hpp)
list(TRANSFORM SOURCES PREPEND "src/")
add_library(libmylisp ${SOURCES})
set_target_properties(libmylisp PROPERTIES LINKER_LANGUAGE CXX)

add_executable(mylisp
  src/main.cpp)
target_link_libraries(mylisp libmylisp)
