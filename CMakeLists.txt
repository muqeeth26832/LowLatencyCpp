cmake_minimum_required(VERSION 3.20)

project(LowLatencyCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(lock-lab)
add_subdirectory(queue-lab)
add_subdirectory(register-lab)
add_subdirectory(scheduling-lab)
add_subdirectory(allocator-lab)
add_subdirectory(active-set-lab)