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
  "main.cpp")
target_link_libraries(step0_repl ${PROJECT_NAME})

add_executable(step1_read_print
  "main.cpp")
target_link_libraries(step1_read_print ${PROJECT_NAME})

add_executable(step2_eval
  "main.cpp")
target_link_libraries(step2_eval ${PROJECT_NAME})

add_executable(step3_env
  "main.cpp")
target_link_libraries(step3_env ${PROJECT_NAME})

add_executable(step4_if_fn_do
  "main.cpp")
target_link_libraries(step4_if_fn_do ${PROJECT_NAME})

add_executable(step5_tco
  "main.cpp")
target_link_libraries(step5_tco ${PROJECT_NAME})

add_executable(step6_file
  "main.cpp")
target_link_libraries(step6_file ${PROJECT_NAME})

add_executable(step7_quote
  "main.cpp")
target_link_libraries(step7_quote ${PROJECT_NAME})

add_executable(step8_macros
  "main.cpp")
target_link_libraries(step8_macros ${PROJECT_NAME})

add_executable(step9_try
  "main.cpp")
target_link_libraries(step9_try ${PROJECT_NAME})

add_executable(stepA_mal
  "main.cpp")
target_link_libraries(stepA_mal ${PROJECT_NAME})

add_executable(mal
  "main.cpp")
target_link_libraries(mal ${PROJECT_NAME})
