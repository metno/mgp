cmake_minimum_required(VERSION 2.8.7)
project (prog_options)
add_executable(po_exec first.cpp)
target_link_libraries(po_exec boost_program_options)
