cmake_minimum_required(VERSION 3.20)

project(QueueLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

find_package(benchmark REQUIRED)
find_package(Threads REQUIRED)

function(add_queue_benchmark target source)
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
        Threads::Threads
    )
endfunction()

add_queue_benchmark(
    single_thread
    benchmarks/single_thread.cpp
)

add_queue_benchmark(
    multi_thread
    benchmarks/multi_thread.cpp
)