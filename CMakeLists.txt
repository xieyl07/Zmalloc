cmake_minimum_required(VERSION 3.25)
project(allocator)

set(CMAKE_CXX_STANDARD 17)

add_executable(allocator allocator_test.cc
        arena.cc
        run.cc)
