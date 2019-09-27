cmake_minimum_required(VERSION 3.14)
project(bupt_cpp_course)

set(CMAKE_CXX_STANDARD 11)
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/array_search/

)

add_executable(array_search  array_search/main.c)
add_executable(hashtable_search  hashtable_search/main.c)
add_executable(bf_search  bf_search/main.c)