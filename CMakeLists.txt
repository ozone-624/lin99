cmake_minimum_required(VERSION 3.15)
project(Lin99Test C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(src)

add_executable(Lin99Test test/test.c)

target_link_libraries(Lin99Test PRIVATE lin99)

set_target_properties(lin99 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set_target_properties(Lin99Test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)