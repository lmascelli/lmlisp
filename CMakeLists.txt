cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Debug")

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME ProjectName)   
string(REPLACE " " "_" PROJECT_NAME ${ProjectName})
project(${PROJECT_NAME} LANGUAGES CXX)

include("cmake/compiler.cmake")

set(SOURCES
  types.cpp
  reader.cpp
  printer.cpp
  core.cpp
  runtime.cpp
  lmlisp.cpp
  )

list(TRANSFORM SOURCES PREPEND "src/")

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
  all_warnings warnings_are_errors)

add_executable(step0_repl
  "step0_repl.cpp")
target_link_libraries(step0_repl ${PROJECT_NAME})

add_executable(step1_read_print
  "step1_read_print.cpp")
target_link_libraries(step1_read_print ${PROJECT_NAME})

add_executable(step2_eval
  "step2_eval.cpp")
target_link_libraries(step2_eval ${PROJECT_NAME})

add_executable(step3_env
  "step3_env.cpp")
target_link_libraries(step3_env ${PROJECT_NAME})
