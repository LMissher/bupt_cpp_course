cmake_minimum_required(VERSION 3.14)
project(bupt_cpp_course)

set(CMAKE_C_FLAGS "-std=c99")

add_executable(btree_search btree_search/main.c)
add_executable(bplus_search bplus_search/main.c)

add_executable(array_search  array_search/main.c)
add_executable(hashtable_search  hashtable_search/main.c)
add_executable(bf_search  bf_search/main.c)
add_executable(patricia_search  patricia_search/main.c)
add_executable(radix4_search  radix4_search/main.c)
add_executable(strstr  strstr/main.c)
add_executable(multikmp  multikmp/main.c)
add_executable(ac_auto  ac_auto/main.c)
add_executable(zjl_cpp zjl_cpp/main.c)