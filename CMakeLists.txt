cmake_minimum_required(VERSION 3.25)
project(spinlockProfile)

set(CMAKE_CXX_STANDARD 20)

add_executable(spinlockProfile main.cpp)
target_include_directories(spinlockProfile PUBLIC src)

add_subdirectory(benchmarks)

message("Thread sanitizer enabled for debug build")
set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fsanitize=thread") # -fsanitize=thread : not working on arm
