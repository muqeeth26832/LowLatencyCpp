cmake_minimum_required(VERSION 3.20)

project(LockLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

find_package(benchmark REQUIRED)

function(add_lock_benchmark target source)
    add_executable(${target} ${source})

    target_include_directories(
        ${target}
        PRIVATE
        include
    )

    target_link_libraries(
        ${target}
        PRIVATE
        benchmark::benchmark
    )

    find_package(Threads REQUIRED)
    target_link_libraries(${target} PRIVATE Threads::Threads)
endfunction()

add_lock_benchmark(
    contention
    benchmarks/contention.cpp
)

add_lock_benchmark(
    list_workload
    benchmarks/list_workload.cpp
)