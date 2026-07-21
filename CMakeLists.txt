cmake_minimum_required(VERSION 3.10)
project(compiler C)
set(CMAKE_C_STANDARD 11)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra)
endif()

file(GLOB SOURCES "src/*.c")
add_executable(compiler ${SOURCES})
target_include_directories(compiler PRIVATE src)
