cmake_minimum_required(VERSION 3.10)
project(ts)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Wall -Wextra")

add_subdirectory(boost-cmake)

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

file(GLOB INCLUDE_H "include/ts/*.h")
file(GLOB INCLUDE_H "include/rs/*.h")
file(GLOB BENCH_INCLUDE_H "*.h")
set(EXAMPLE_FILES example.cc)
set(BENCH_END_TO_END_FILES bench_end_to_end.cc)

add_executable(example ${INCLUDE_H} ${EXAMPLE_FILES})
target_link_libraries(example PUBLIC Boost::serialization)
target_link_libraries(example PUBLIC Boost::iostreams)

add_executable(bench_end_to_end ${INCLUDE_H} ${BENCH_INCLUDE_H} ${BENCH_END_TO_END_FILES})
target_link_libraries(bench_end_to_end PUBLIC Boost::serialization)
target_link_libraries(bench_end_to_end PUBLIC Boost::iostreams)

add_executable(kv_build ${INCLUDE_H} ${BENCH_INCLUDE_H} kv_build.cc)
target_link_libraries(kv_build PUBLIC Boost::serialization)
target_link_libraries(kv_build PUBLIC Boost::iostreams)

add_executable(kv_benchmark ${INCLUDE_H} ${BENCH_INCLUDE_H} kv_benchmark.cc)
target_link_libraries(kv_benchmark PUBLIC Boost::serialization)
target_link_libraries(kv_benchmark PUBLIC Boost::iostreams)
